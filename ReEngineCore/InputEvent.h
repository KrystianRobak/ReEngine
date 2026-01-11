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

    int key;
    MouseButton mouseBtn;
    KeyState state;

    double x, y;
    double scroll;
};
