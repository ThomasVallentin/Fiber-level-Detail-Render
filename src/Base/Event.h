#ifndef EVENT_H
#define EVENT_H

#include "Inputs.h"

#define DEFINE_EVENT_CATEGORY(_type) inline EventCategory GetCategory() const override { return EventCategory::_type; }
#define DEFINE_EVENT_TYPE(_type) inline uint32_t GetType() const override { return EventType::_type; }  \
                                 inline const char* GetName() const override { return #_type; }   

enum class EventCategory {
    None = 0,
    Window,
    Input,
};

enum EventType {
    None = 0,
    WindowResized, WindowClosed,
    KeyPressed, KeyReleased, 
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};


class Event {
public:
    virtual EventCategory GetCategory() const = 0;
    virtual uint32_t GetType() const = 0;
    virtual const char* GetName() const = 0;
};


// == Window Events ==


class WindowResizedEvent : public Event {
public:
    WindowResizedEvent(const float& width, const float& height) : 
            m_width(width), m_height(height) {}

    inline float GetWidth() const { return m_width; }
    inline float GetHeight() const { return m_height; }

    DEFINE_EVENT_CATEGORY(Window)
    DEFINE_EVENT_TYPE(WindowResized)

private:
    float m_width;
    float m_height;
};


class WindowClosedEvent : public Event {
public:
    WindowClosedEvent() = default;

    DEFINE_EVENT_CATEGORY(Window)
    DEFINE_EVENT_TYPE(WindowClosed)
};


// == Key Events ==


class KeyEvent : public Event {
public:
    explicit KeyEvent(const KeyCode& key, const int& modifiers) : 
            m_key(key), m_modifiers(modifiers) {}

    inline KeyCode GetKey() const { return m_key; }
    inline int GetModifiers() const { return m_modifiers; }

    DEFINE_EVENT_CATEGORY(Input)

private:
    KeyCode m_key;
    int m_modifiers;
};


class KeyPressedEvent : public KeyEvent {
public:
    explicit KeyPressedEvent(const KeyCode& key, const int& modifiers) : 
            KeyEvent(key, modifiers) {}

    DEFINE_EVENT_TYPE(KeyPressed)
};


class KeyReleasedEvent : public KeyEvent {
public:
    explicit KeyReleasedEvent(const KeyCode& key, const int& modifiers) : 
            KeyEvent(key, modifiers) {}

    DEFINE_EVENT_TYPE(KeyReleased)
};


// == Mouse Events ==


class MouseButtonEvent : public Event {
public:
    explicit MouseButtonEvent(const MouseButton& button, const int& modifiers) : 
            m_button(button), m_modifiers(modifiers) {}

    inline MouseButton GetButton() const { return m_button; }
    inline int GetModifiers() const { return m_modifiers; }

    DEFINE_EVENT_CATEGORY(Input)

private:
    MouseButton m_button;
    int m_modifiers;
};


class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    explicit MouseButtonPressedEvent(const MouseButton& button, const int& modifiers) : 
            MouseButtonEvent(button, modifiers) {}

    DEFINE_EVENT_TYPE(MouseButtonPressed)
};


class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    explicit MouseButtonReleasedEvent(const MouseButton& button, const int& modifiers) : 
            MouseButtonEvent(button, modifiers) {}

    DEFINE_EVENT_TYPE(MouseButtonReleased)
};


class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(const float& posX, const float& posY) : 
            m_posX(posX), m_posY(posY) {}

    inline float GetPosX() const { return m_posX; }
    inline float GetPosY() const { return m_posY; }

    DEFINE_EVENT_CATEGORY(Input)
    DEFINE_EVENT_TYPE(MouseMoved)

private:
    float m_posX;
    float m_posY;
};


class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(const float& offsetX, const float& offsetY) : 
            m_offsetX(offsetX), m_offsetY(offsetY) {}

    inline float GetOffsetX() const { return m_offsetX; }
    inline float GetOffsetY() const { return m_offsetY; }

    DEFINE_EVENT_CATEGORY(Input)
    DEFINE_EVENT_TYPE(MouseScrolled)

private:
    float m_offsetX;
    float m_offsetY;
};


#endif  // EVENT_H
