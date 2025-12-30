#pragma once
#include "InputEnums.h"

using ActionID = size_t;
using ActionCallback = std::function<void()>;


struct InputEvent
{
    enum class Type
    {
        Key,
        MouseButton,
        MouseMove,
        Scroll
    } type;

    int key;               // GLFW key
    MouseButton mouseBtn;  // Mouse buttons
    KeyState state;

    double x, y;           // Mouse pos or delta
    double scroll;
};
