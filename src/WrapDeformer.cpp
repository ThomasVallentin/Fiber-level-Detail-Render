#include "WrapDeformer.h"

#include "Base/Math.h"

#include <glm/gtx/string_cast.hpp>

#include <omp.h>


WrapDeformer::WrapDeformer()
{

}

WrapDeformer::~WrapDeformer()
{

}

void WrapDeformer::Initialize(const std::vector<glm::vec3>& points, 
                              const std::vector<Vertex>& driverVertices, 
                              const std::vector<uint32_t>& driverIndices)
{
    m_bindings.clear();
    m_bindings.reserve(points.size());
    m_coordinates.clear();
    m_coordinates.reserve(points.size());

    for (const auto& point : points)
    {
        uint32_t triangleIndex;
        float distance;
        Mesh::ClosestPointOnMesh(point, driverVertices, driverIndices, triangleIndex, distance);

        glm::vec3 p1 = driverVertices[driverIndices[triangleIndex * 3]].position;
        glm::vec3 p2 = driverVertices[driverIndices[triangleIndex * 3 + 1]].position;
        glm::vec3 p3 = driverVertices[driverIndices[triangleIndex * 3 + 2]].position;
        glm::vec3 normal = glm::normalize(glm::cross(p1 - p2, p2 - p3));

        glm::vec3 projectedPoint = ProjectPointOnPlane(point, normal, p1);
        glm::vec3 barycentric = CartesianToBarycentric(projectedPoint, p1, p2, p3);
        float normalOffset = glm::distance(point, projectedPoint);
        if (glm::dot(normal, point - projectedPoint) < 0.0)
            normalOffset *= -1;

        m_bindings.push_back(triangleIndex);
        m_coordinates.push_back({barycentric.x, barycentric.y, barycentric.z, normalOffset});
    }

    m_restPoints = points;
}

void WrapDeformer::Deform(std::vector<glm::vec3>& points, 
                          const std::vector<Vertex>& driverVertices, 
                          const std::vector<uint32_t>& driverIndices) const
{
    if (m_bindings.empty())
        return;

    #pragma omp parallel for num_threads(omp_get_max_threads())
    for (uint32_t i = 0 ; i < points.size() ; i++)
    {
        const uint32_t& triangleIndex = m_bindings[i];
        const glm::vec3 barycentric = m_coordinates[i];
        const float normalOffset = m_coordinates[i].w;

        const glm::vec3& v1 = driverVertices[driverIndices[triangleIndex * 3]].position;
        const glm::vec3& v2 = driverVertices[driverIndices[triangleIndex * 3 + 1]].position;
        const glm::vec3& v3 = driverVertices[driverIndices[triangleIndex * 3 + 2]].position;

        const glm::vec3 normal = glm::normalize(glm::cross(v1 - v2, v2 - v3));

        points[i] = BarycentricToCartesian(barycentric, v1, v2, v3) + normal * normalOffset;
    }

    ApplySmooth(points);
}


void WrapDeformer::ApplySmooth(std::vector<glm::vec3>& points) const
{
    if (m_iterations <= 0)
        return;

    std::vector<glm::vec3> deltas = points;
    std::vector<glm::vec3> deltas2(points.size());
    for (size_t i = 0 ; i < points.size() ; i++)
        deltas[i] -= m_restPoints[i];
        
    auto* sources = &deltas;
    auto* smoothed = &deltas2;
    for (size_t n = 0 ; n < m_iterations ; n++)
    {
        for (size_t i = 0 ; i < points.size() ; i++)
        {
            glm::vec3 delta = (*sources)[i];
            float weight = 1.0f;

            if (i > 0)
            {
                delta += deltas[i - 1];
                weight += 1.0f;
            }

            if (i < points.size())
            {
                delta += deltas[i + 1];
                weight += 1.0f;
            }

            (*smoothed)[i] = delta / weight;
        }

        std::swap(smoothed, sources);
    }

    for (size_t i = 0 ; i < points.size() ; i++)
        points[i] = m_restPoints[i] + (*sources)[i];
        
}