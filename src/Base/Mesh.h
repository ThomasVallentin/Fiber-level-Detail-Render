#ifndef MESH_H
#define MESH_H


#include <glm/glm.hpp>

#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};


namespace Mesh {


void BuildPlane(const float& width, 
                const float& height, 
                const uint32_t& divisionsW, 
                const uint32_t& divisionsH, 
                std::vector<Vertex>& vertices, 
                std::vector<uint32_t>& indices);

void GenerateNormals(std::vector<Vertex>& vertices,
                     const std::vector<uint32_t>& indices);

glm::vec3 ClosestPointOnMesh(const glm::vec3& p,
                            const std::vector<Vertex>& vertices,
                            const std::vector<uint32_t>& indices,
                            uint32_t& triangleIndex,
                            float &distance);


}  // namespace Mesh

#endif