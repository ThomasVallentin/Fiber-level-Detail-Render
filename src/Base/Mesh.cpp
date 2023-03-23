#include "Mesh.h"
#include "Math.h"

#include <omp.h>


namespace Mesh {


void BuildPlane(const float& width, 
                const float& height, 
                const uint32_t& divisionsW, 
                const uint32_t& divisionsH, 
                std::vector<Vertex>& vertices, 
                std::vector<uint32_t>& indices)
{
    vertices.clear();
    indices.clear();
    int index = 0;
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    for (int i = 0 ; i < (divisionsH + 1) ; ++i)
    {
        for (int j = 0 ; j < (divisionsW + 1) ; ++j)
        {
            vertices.push_back({{(float)j / divisionsW * width - halfWidth, 
                                (float)i / divisionsH * height - halfHeight, 0.0f}, 
                               {0.0, 0.0, 1.0}, 
                               {(float)j / divisionsW, 
                                (float)i / divisionsH}});

            if ((i <= 0) || (j >= divisionsW)) {
                index++;
                continue;
            }

            int downIndex = index - divisionsW - 1;
            int diagDownIndex = downIndex + 1;
            int sideIndex = index + 1;

            indices.push_back(index); 
            indices.push_back(downIndex);
            indices.push_back(diagDownIndex);
            indices.push_back(diagDownIndex); 
            indices.push_back(sideIndex);
            indices.push_back(index);

            index++;
        }
    }
}



void GenerateNormals(std::vector<Vertex>& vertices,
                     const std::vector<uint32_t>& indices) {
    for (auto& vertex : vertices) {
        vertex.normal = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < indices.size() ; i += 3)
    {
        Vertex& v1 = vertices[indices[i]];
        Vertex& v2 = vertices[indices[i + 1]];
        Vertex& v3 = vertices[indices[i + 2]];

        auto normal = glm::normalize(glm::cross(v1.position - v2.position, 
                                                v2.position - v3.position));
        v1.normal += normal;
        v2.normal += normal;
        v3.normal += normal;
    }

    for (auto& vertex : vertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }
}


glm::vec3 ClosestPointOnMesh(const glm::vec3& p,
                             const std::vector<Vertex>& vertices,
                             const std::vector<uint32_t>& indices,
                             uint32_t& triangleIndex,
                             float &distance)
{
    int threadCount = omp_get_max_threads();

    glm::vec3 closestPoints[threadCount];
    uint32_t closestIndices[threadCount];
    float closestDistances[threadCount];
    for (int i = 0 ; i < threadCount ; i++)
        closestDistances[i] = std::numeric_limits<float>::max();

    uint32_t triangleCount = indices.size() / 3;
    #pragma omp parallel for num_threads(threadCount)
    for (auto idx = 0 ; idx < triangleCount ; idx++ )
    {
        int threadnum = omp_get_thread_num();
        glm::vec3 currPoint = ClosestPointOnTriangle(p, 
                                                     vertices[indices[idx * 3]].position, 
                                                     vertices[indices[idx * 3 + 1]].position,
                                                     vertices[indices[idx * 3 + 2]].position);
        float currDistance = glm::distance(p, currPoint);
        if (currDistance < closestDistances[threadnum])
        {
            closestDistances[threadnum] = currDistance;
            closestPoints[threadnum] = currPoint;
            closestIndices[threadnum] = idx;
        }
    }

    float closestDistance = std::numeric_limits<float>::max();
    uint32_t index;
    for (int i = 0 ; i < threadCount ; i++)
    {
        if (closestDistances[i] < closestDistance)
        {
            closestDistance = closestDistances[i];
            index = i;
        }

    }

    distance = closestDistances[index];
    triangleIndex = closestIndices[index];
    return closestPoints[index];
}   


}  // namespace Mesh
