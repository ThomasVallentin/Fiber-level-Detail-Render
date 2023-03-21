#include "DirectionalLight.h"

#include <glm/gtc/matrix_transform.hpp>

DirectionalLight::DirectionalLight()
{
    UpdateView();
    UpdateProjection();
}

DirectionalLight::DirectionalLight(const glm::vec3& color, const float& intensity) :
        m_color(color),
        m_intensity(intensity)
{
    UpdateView();
    UpdateProjection();
}

void DirectionalLight::SetDirection(const glm::vec3& direction) 
{
    m_direction = direction;
    UpdateView();
}

void DirectionalLight::SetProjSize(const float& size)
{
    m_projSize = size;
    UpdateProjection();
}

void DirectionalLight::SetClipPlanes(const float& nearClip, const float& farClip)
{
    m_nearClip = nearClip;
    m_farClip = farClip;
    UpdateView();
    UpdateProjection();
}

void DirectionalLight::UpdateView() 
{
    m_viewMatrix = glm::lookAt(-m_direction * m_farClip * 0.5f, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
}

void DirectionalLight::UpdateProjection() 
{
    m_projectionMatrix = glm::ortho(-m_projSize, m_projSize, -m_projSize, m_projSize, m_nearClip, m_farClip); 
}
