#ifndef CAMERA_H
#define CAMERA_H

#include "Event.h"

#include <glm/gtx/quaternion.hpp>


//
// The Camera class is a simple trackball camera controller that offers Maya-like camera controls :
// - Pan    : Alt + Middle click + Mouse move
// - Rotate : Alt + Left click   + Mouse move
// - Zoom   : Alt + Right click  + Mouse move (horizontal)
//
class Camera {
public:
    Camera();
    Camera(const float &fov,
           const float &aspectRatio,
           const float &nearClip,
           const float &farClip);

    void Update();
    void OnEvent(Event* event);

    void SetViewportSize(const float &width, const float &height);

    glm::vec3 GetForwardDirection() const;
    glm::vec3 GetUpDirection() const;
    glm::vec3 GetSideDirection() const;
    glm::quat GetOrientation() const;
    glm::vec3 GetEulerAngles() const;
    float GetFov() const;
    inline const glm::vec3& GetPosition() const { return m_position; }

    inline const glm::mat4& GetViewMatrix() { return m_viewMatrix; };
    inline const glm::mat4& GetProjectionMatrix() { return m_projectionMatrix; };
    inline glm::mat4 GetViewProjectionMatrix() const { return m_projectionMatrix * m_viewMatrix; }

private:
    glm::vec3 ComputePosition() const;
    void UpdateView();
    void UpdateProjection();

    void MousePan(const float &deltaX, const float &deltaY);
    void MouseRotate(const float &deltaX, const float &deltaY);
    void MouseZoom(const float &deltaX);

    bool OnMouseScrollEvent(MouseScrolledEvent* event);
    bool OnKeyPressedEvent(KeyPressedEvent* event);

    glm::vec2 m_prevMousePos;

    float m_viewportWidth = 1280;
    float m_viewportHeight = 720;

    float m_fov = 45.0f;
    float m_aspectRatio = 1.778f;
    float m_nearClip = 0.1f;
    float m_farClip = 1000.0f;

    float m_distance = 5.0f;
    glm::vec3 m_position { 0.0f, 0.0f, 5.0f};
    glm::vec3 m_focalPoint { 0.0f, 0.0f, 0.0f};

    float m_pitch = -0.5f;
    float m_yaw = 0.5f;
    float m_roll = 0.0f;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;
};


#endif // CAMERA_H