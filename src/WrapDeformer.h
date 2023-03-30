#ifndef WRAPDEFORMER_H
#define WRAPDEFORMER_H


#include "Base/Mesh.h"

#include <glm/glm.hpp>


class WrapDeformer
{
public:
    WrapDeformer();
    ~WrapDeformer();

    inline bool IsInitialized() const { return (!m_bindings.empty() || !m_coordinates.empty() || !m_restPoints.empty()); }
    void Initialize(const std::vector<glm::vec3>& points, const std::vector<Vertex>& srcVertices, const std::vector<uint32_t>& srcIndices);
    void Deform(std::vector<glm::vec3>& points, const std::vector<Vertex>& srcVertices, const std::vector<uint32_t>& srcIndices) const;

    inline uint32_t GetSmoothIterations() const { return m_iterations; }
    inline void SetSmoothIterations(const uint32_t& count) { m_iterations = count; }

private:
    void ApplySmooth(std::vector<glm::vec3>& points) const;

    std::vector<uint32_t> m_bindings;
    std::vector<glm::vec4> m_coordinates;

    std::vector<glm::vec3> m_restPoints;
    uint32_t m_iterations = 3;
};

#endif  // WRAPDEFORMER_H