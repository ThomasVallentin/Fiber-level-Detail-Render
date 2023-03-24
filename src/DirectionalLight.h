#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H


#include <glm/glm.hpp>


class DirectionalLight
{
public:
    DirectionalLight();
    DirectionalLight(const glm::vec3& color, const float& intensity=1.0f);
    DirectionalLight(const glm::vec3& direction, 
                     const glm::vec3& color, 
                     const float& intensity=1.0f);

    void SetDirection(const glm::vec3& direction);
    void SetProjSize(const float& size);
    void SetClipPlanes(const float& nearClip, const float& farClip);

    inline glm::vec3 GetPower() const { return m_color * m_intensity; }
    inline glm::mat4 GetViewMatrix() const { return m_viewMatrix; }
    inline glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }

private:
    void UpdateView();
    void UpdateProjection();

    float m_intensity = 1.0;
    glm::vec3 m_color = {1.0f, 1.0f, 1.0f};

    float m_nearClip =  0.1f;
    float m_farClip  = 20.0f;
    float m_projSize = 10.0f;

    glm::vec3 m_direction = {0.0f, 0.0f, 1.0f};
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;
};

#endif