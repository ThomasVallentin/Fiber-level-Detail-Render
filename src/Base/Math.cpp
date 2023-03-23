#include "Math.h"

#include <glm/gtx/string_cast.hpp>

#include <omp.h>

#include <algorithm>
#include <iostream>


glm::vec3 CartesianToBarycentric(const glm::vec3& p, const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3)
{
    glm::vec3 v0 = t2 - t1;
    glm::vec3 v1 = t3 - t1;
    glm::vec3 v2 = t1 - p;

    float a = glm::dot(v0, v0);
    float b = glm::dot(v0, v1);
    float c = glm::dot(v1, v1);
    float d = glm::dot(v0, v2);
    float e = glm::dot(v1, v2);
    float det = a * c - b * b;

    float v = (c * d - b * e) / det;
    float w = (a * e - b * d) / det;
    float u = 1.0f - v - w;
    return {u, v, w};
}

glm::vec3 BarycentricToCartesian(const glm::vec3& barycentricCoords, const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3)
{
    return barycentricCoords.x * t1 + barycentricCoords.y * t2 + barycentricCoords.z * t3;
}

// From https://www.geometrictools.com/Documentation/DistancePoint3Triangle3.pdf
glm::vec3 ClosestPointOnTriangle(const glm::vec3& p,
                                 const glm::vec3& t1, 
                                 const glm::vec3& t2, 
                                 const glm::vec3& t3)
{
    glm::vec3 v0 = t2 - t1;
    glm::vec3 v1 = t3 - t1;
    glm::vec3 v2 = t1 - p;

    float a = glm::dot(v0, v0);
    float b = glm::dot(v0, v1);
    float c = glm::dot(v1, v1);
    float d = glm::dot(v0, v2);
    float e = glm::dot(v1, v2);

    float det = a * c - b * b;
    float s = b * e - c * d;
    float t = b * d - a * e;

    // If the We encounter 6 regions where the 
    if (s + t < det)
        if (s < 0.0f)
            if (t < 0.0f)
                if (d < 0.0f) {
                    s = std::clamp(-d / a, 0.0f, 1.0f);
                    t = 0.0f;
                } else {
                    s = 0.0f;
                    t = std::clamp(-e / c, 0.0f, 1.0f);
                }
            else {
                s = 0.0f;
                t = std::clamp(-e / c, 0.0f, 1.0f);
            }
        else if (t < 0.0f) {
            s = std::clamp(-d / a, 0.0f, 1.0f);
            t = 0.0f;
        } else {
            float invDet = 1.0f / det;
            s *= invDet;
            t *= invDet;
        }
    else
        if (s < 0.0f) {
            float tmp0 = b + d;
            float tmp1 = c + e;
            if (tmp1 > tmp0) {
                float num = tmp1 - tmp0;
                float denom = a-2*b+c;
                s = std::clamp(num / denom, 0.0f, 1.0f);
                t = 1 - s;
            } else {
                t = std::clamp(-e/c, 0.0f, 1.0f);
                s = 0.0f;
            }
        } else if (t < 0.0f) {
            if (a + d > b + e) {
                float num = c + e - b - d;
                float denom = a - 2 * b + c;
                s = std::clamp(num / denom, 0.0f, 1.0f);
                t = 1 - s;
            } else {
                s = std::clamp(-e / c, 0.0f, 1.0f);
                t = 0.0f;
            }
        } else {
            float num = c + e - b - d;
            float denom = a - 2 * b + c;
            s = std::clamp(num / denom, 0.0f, 1.0f);
            t = 1.0f - s;
        }

    return t1 + s * v0 + t * v1;
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
