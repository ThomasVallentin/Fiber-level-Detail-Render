#ifndef INPUTS_H
#define INPUTS_H

#include <glm/vec2.hpp>

// These enums are just a 1 to 1 copy of the inputs used in GLFW.
// The same goes for the various functions of the Inputs class.
// Copying them allow us to avoid including GLFW every time we need an input
// and enhance readability.

enum class KeyCode
{   
    Digit_0          = 48,
    Digit_1          = 49,
    Digit_2          = 50,
    Digit_3          = 51,
    Digit_4          = 52,
    Digit_5          = 53,
    Digit_6          = 54,
    Digit_7          = 55,
    Digit_8          = 56,
    Digit_9          = 57,

    A                = 65,
    B                = 66,
    C                = 67,
    D                = 68,
    E                = 69,
    F                = 70,
    G                = 71,
    H                = 72,
    I                = 73,
    J                = 74,
    K                = 75,
    L                = 76,
    M                = 77,
    N                = 78,
    O                = 79,
    P                = 80,
    Q                = 81,
    R                = 82,
    S                = 83,
    T                = 84,
    U                = 85,
    V                = 86,
    W                = 87,
    X                = 88,
    Y                = 89,
    Z                = 90,

    Space            = 32,
    Escape           = 256,
    Enter            = 257,
    Tab              = 258,
    Backspace        = 259,
    Insert           = 260,
    Delete           = 261,

    Right            = 262,
    Left             = 263,
    Down             = 264,
    Up               = 265,

    F1               = 290,
    F2               = 291,
    F3               = 292,
    F4               = 293,
    F5               = 294,
    F6               = 295,
    F7               = 296,
    F8               = 297,
    F9               = 298,
    F10              = 299,
    F11              = 300,
    F12              = 301,
    F13              = 302,
    F14              = 303,
    F15              = 304,
    F16              = 305,
    F17              = 306,
    F18              = 307,
    F19              = 308,
    F20              = 309,
    F21              = 310,
    F22              = 311,
    F23              = 312,
    F24              = 313,
    F25              = 314,

    KeyPad_0         = 320,
    KeyPad_1         = 321,
    KeyPad_2         = 322,
    KeyPad_3         = 323,
    KeyPad_4         = 324,
    KeyPad_5         = 325,
    KeyPad_6         = 326,
    KeyPad_7         = 327,
    KeyPad_8         = 328,
    KeyPad_9         = 329,
    KeyPad_Decimal   = 330,
    KeyPad_Divide    = 331,
    KeyPad_Multiply  = 332,
    KeyPad_Subtract  = 333,
    KeyPad_Add       = 334,
    KeyPad_Enter     = 335,
    KeyPad_Equal     = 336,

    CapsLock         = 280,
    LeftShift        = 340,
    LeftCtrl         = 341,
    LeftAlt          = 342,
    RightShift       = 344,
    RightCtrl        = 345,
    RightAlt         = 346,
};


enum class MouseButton
{
    Button_1  = 0,
    Button_2  = 1,
    Button_3  = 2,
    Button_4  = 3,
    Button_5  = 4,
    Button_6  = 5,
    Button_7  = 6,
    Button_8  = 7,

    Left      = Button_1,
    Right     = Button_2,
    Middle    = Button_3
};


class Inputs
{
public:
    static bool IsKeyPressed(const KeyCode &key);
    static bool IsMouseButtonPressed(const MouseButton &button);
    static glm::vec2 GetMousePosition();
};


#endif // INPUTS_H
