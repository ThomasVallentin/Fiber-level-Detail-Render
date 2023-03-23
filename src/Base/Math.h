#ifndef MATH_H
#define MATH_H


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


// Projection
glm::vec3 ProjectPointOnPlane(const glm::vec3& point, const glm::vec3& normal, const glm::vec3& planePoint);

#endif // MATH_H