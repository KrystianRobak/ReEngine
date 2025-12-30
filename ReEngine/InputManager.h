#pragma once

#include "InputManagerApi.h"
#include <mutex>
#include <vector>
#include <unordered_map>
#include <functional>


constexpr ActionID HashString(const char* str) {
    size_t hash = 14695981039346656037ULL;
    while (*str) hash = (hash ^ static_cast<size_t>(*str++)) * 1099511628211ULL;
    return hash;
}

class InputManager : public IInputManager
{
public:

    // --- CALLED BY RENDER THREAD (GLFW/Window) ---
    void PushEvent(const InputEvent& event) override;


    // --- CALLED BY LOGIC THREAD (User/Game Loop) ---

    // 1. Setup: Map a hardware key to an Action Name (e.g., Space -> "Jump")
    void BindKey(const std::string& actionName, int glfwKey) override;
    void BindMouse(const std::string& actionName, MouseButton button) override;

    // 2. Setup: Bind a function to an Action Name
    void BindAction(const std::string& actionName, InputActionState triggerState, ActionCallback callback) override;

    // 3. Polling: Check if an action is currently held down (good for movement)
    bool IsActionActive(const std::string& actionName) override;
    bool IsActionActive(ActionID actionId) override; // Faster version

    void Update(float dt);


    InputManager();

private:
    struct ActionBinding
    {
        InputActionState triggerState;
        ActionCallback callback;
    };

    // Thread Safety
    std::mutex m_eventMutex;
    std::vector<InputEvent> m_pendingEvents; // Incoming from Render Thread
    std::vector<InputEvent> m_frameEvents;   // Processing on Logic Thread

    // State Tracking
    std::unordered_map<int, KeyState> m_keyStates;
    std::unordered_map<MouseButton, KeyState> m_mouseStates;

    // Action Mappings (Using Hashed IDs for speed)
    std::unordered_map<ActionID, int> m_keyMap;
    std::unordered_map<ActionID, MouseButton> m_mouseMap;
    std::unordered_map<ActionID, std::vector<ActionBinding>> m_actionBindings;
};

