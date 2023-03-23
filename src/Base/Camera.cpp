#include "Camera.h"

#include "Logging.h"
#include "Inputs.h"


Camera::Camera()
{
    UpdateProjection();
    UpdateView();
}

Camera::Camera(const float &fov, const float &aspectRatio, const float &nearClip,
                const float &farClip)
        : m_fov(fov),
            m_aspectRatio(aspectRatio),
            m_nearClip(nearClip),
            m_farClip(farClip) 
{
    UpdateProjection();
    UpdateView();
}

void Camera::Update() {
    glm::vec2 mousePos = Inputs::GetMousePosition();

    // Only editing the camera when alt is pressed
    if (Inputs::IsKeyPressed(KeyCode::LeftAlt)) {
        glm::vec2 delta = mousePos - m_prevMousePos;

        if (Inputs::IsMouseButtonPressed( MouseButton::Middle))
            MousePan(delta.x, delta.y);
        else if (Inputs::IsMouseButtonPressed(MouseButton::Left))
            MouseRotate(delta.x, delta.y);
        else if (Inputs::IsMouseButtonPressed(MouseButton::Right))
            MouseZoom(delta.x);
    }

    m_prevMousePos = mousePos;
}

void Camera::MousePan(const float &deltaX, const float &deltaY) {
    m_focalPoint -= GetSideDirection() * deltaX * 3.0f * m_distance / m_viewportWidth;
    m_focalPoint += GetUpDirection() * deltaY * 3.0f * m_distance / m_viewportWidth;

    UpdateView();
}

void Camera::MouseRotate(const float &deltaX, const float &deltaY) {
    m_yaw -= deltaX * 0.01f;
    m_pitch -= deltaY * 0.01f;
    UpdateView();
}

void Camera::MouseZoom(const float &deltaX) {
    m_distance -= deltaX * 0.003f * m_distance;
    UpdateView();
}

void Camera::OnEvent(Event* event) {
    switch (event->GetType())
    {
        case EventType::MouseScrolled: {
            Camera::OnMouseScrollEvent(dynamic_cast<MouseScrolledEvent*>(event));
            return;
        }
        case EventType::KeyPressed: {
            Camera::OnKeyPressedEvent(dynamic_cast<KeyPressedEvent*>(event));
            return;
        }
    }
}

bool Camera::OnMouseScrollEvent(MouseScrolledEvent* event) {
    m_distance -= event->GetOffsetY() * 0.3f * m_distance;

    UpdateView();

    return false;
}

bool Camera::OnKeyPressedEvent(KeyPressedEvent* event) {
    // Pseudo focus that reset the camera distance and focal point
    if (event->GetKey() == KeyCode::F) {
        m_focalPoint = {0.0f, 0.0f, 0.0f};
        m_distance = 5.0f;
        m_pitch = -0.5f;
        m_yaw = 0.5f;

        UpdateView();
    }

    return false;
}

void Camera::SetViewportSize(const float &width, const float &height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
    m_aspectRatio = m_viewportWidth / m_viewportHeight;
    UpdateProjection();
}

glm::vec3 Camera::GetForwardDirection() const {
    return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 Camera::GetUpDirection() const {
    return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 Camera::GetSideDirection() const {
    return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::quat Camera::GetOrientation() const {
    return {glm::vec3(m_pitch, m_yaw, m_roll) };
}

glm::vec3 Camera::ComputePosition() const {
    return m_focalPoint - GetForwardDirection() * m_distance;
}

void Camera::UpdateView() {
    auto oldMatrix = m_viewMatrix;

    m_position = ComputePosition();
    glm::quat orientation = GetOrientation();
    m_viewMatrix = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(orientation);
    m_viewMatrix = glm::inverse(m_viewMatrix);
}

void Camera::UpdateProjection() {
    auto oldMatrix = m_projectionMatrix;

    m_projectionMatrix = glm::perspective(glm::radians(m_fov),
                                          m_aspectRatio,
                                          m_nearClip,
                                          m_farClip);
}

float Camera::GetFov() const {
    return m_fov;
}

glm::vec3 Camera::GetEulerAngles() const {
    return {m_pitch, m_yaw, m_roll};
}
