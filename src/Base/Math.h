#ifndef MATH_H
#define MATH_H


#include "VertexArray.h"

#include <glm/glm.hpp>


// Barycentric coordinates
glm::vec3 CartesianToBarycentric(const glm::vec3& p, 
                                 const glm::vec3& a, 
                                 const glm::vec3& b, 
                                 const glm::vec3& c);
glm::vec3 BarycentricToCartesian(const glm::vec3& barycentricCoords, 
                                 const glm::vec3& a, 
                                 const glm::vec3& b, 
                                 const glm::vec3& c);
glm::vec3 ClosestPointOnTriangle(const glm::vec3& p,
                                 const glm::vec3& t1, 
                                 const glm::vec3& t2, 
                                 const glm::vec3& t3);
glm::vec3 ClosestPointOnMesh(const glm::vec3& p,
                             const std::vector<Vertex>& vertices,
                             const std::vector<uint32_t>& indices,
                             uint32_t& triangleIndex,
                             float &distance);

#endif // MATH_H