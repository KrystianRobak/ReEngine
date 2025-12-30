#pragma once

#include <string>
#include <functional>
#include "InputEvent.h"

class IInputManager {
public:
    virtual ~IInputManager() = default;

    // --- CALLED BY RENDER THREAD (GLFW/Window) ---
    virtual void PushEvent(const InputEvent& event) = 0;

    virtual void BindKey(const std::string& actionName, int glfwKey) = 0;
    virtual void BindMouse(const std::string& actionName, MouseButton button) = 0;

    // 2. Setup: Bind a function to an Action Name
    virtual void BindAction(const std::string& actionName, InputActionState triggerState, ActionCallback callback) = 0;

    // 3. Polling: Check if an action is currently held down (good for movement)
    virtual bool IsActionActive(const std::string& actionName) = 0;
    virtual bool IsActionActive(ActionID actionId) = 0;
};