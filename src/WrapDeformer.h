#ifndef WRAPDEFORMER_H
#define WRAPDEFORMER_H


#include "Base/Mesh.h"

#include <glm/glm.hpp>


class WrapDeformer
{
public:
    WrapDeformer();
    ~WrapDeformer();

    void Initialize(const std::vector<glm::vec3>& points, const std::vector<Vertex>& srcVertices, const std::vector<uint32_t>& srcIndices);
    void Deform(std::vector<glm::vec3>& points, const std::vector<Vertex>& srcVertices, const std::vector<uint32_t>& srcIndices) const;

private:
    std::vector<uint32_t> m_bindings;
    std::vector<glm::vec4> m_coordinates;
};

#endif  // WRAPDEFORMER_H