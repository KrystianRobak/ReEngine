#include "InputManager.h"

InputManager::InputManager()
{
    // Reserve memory to prevent reallocations during gameplay
    m_pendingEvents.reserve(100);
    m_frameEvents.reserve(100);
}

// ---------------------------------------------------------
// RENDER THREAD SECTION
// ---------------------------------------------------------
void InputManager::PushEvent(const InputEvent& event)
{
    // Lock briefly to safely add the event
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_pendingEvents.push_back(event);
}

// ---------------------------------------------------------
// LOGIC THREAD SECTION
// ---------------------------------------------------------

void InputManager::BindKey(const std::string& action, int glfwKey)
{
    m_keyMap[HashString(action.c_str())] = glfwKey;
}

void InputManager::BindMouse(const std::string& action, MouseButton button)
{
    m_mouseMap[HashString(action.c_str())] = button;
}

void InputManager::BindAction(const std::string& action, InputActionState state, ActionCallback callback)
{
    m_actionBindings[HashString(action.c_str())].push_back({ state, callback });
}

bool InputManager::IsActionActive(const std::string& action)
{
    return IsActionActive(HashString(action.c_str()));
}

bool InputManager::IsActionActive(ActionID actionId)
{
    // Check Keys
    auto kIt = m_keyMap.find(actionId);
    if (kIt != m_keyMap.end())
    {
        auto ks = m_keyStates.find(kIt->second);
        // "Held" usually implies the button is down. 
        // Depending on your GLFW wrapper, Pressed might need to be checked too.
        return ks != m_keyStates.end() && (ks->second == KeyState::Held || ks->second == KeyState::Pressed);
    }

    // Check Mouse
    auto mIt = m_mouseMap.find(actionId);
    if (mIt != m_mouseMap.end())
    {
        auto ms = m_mouseStates.find(mIt->second);
        return ms != m_mouseStates.end() && (ms->second == KeyState::Held || ms->second == KeyState::Pressed);
    }

    return false;
}

std::pair<float, float> InputManager::GetMousePosition()
{
    return { m_currMouseX, m_currMouseY };
}

void InputManager::Update(float dt)
{
    // 1. SWAP BUFFERS (Critical Section)
    // We move all pending events to our local frame list so we can release the lock immediately.
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        m_frameEvents = std::move(m_pendingEvents);
        m_pendingEvents = std::vector<InputEvent>(); // Reset pending
        m_pendingEvents.reserve(100);
    }

    // 2. PROCESS EVENTS (Thread Safe / Local)
    for (const auto& event : m_frameEvents)
    {
        // Update Internal State Maps
        switch (event.type)
        {
        case InputEvent::Type::Key:
            m_keyStates[event.key] = event.state;
            break;
        case InputEvent::Type::MouseButton:
            m_mouseStates[event.mouseBtn] = event.state;
            break;
		case InputEvent::Type::MouseMove:
            m_currMouseX = static_cast<float>(event.x);
            m_currMouseY = static_cast<float>(event.y);
			break;
        default: break;
        }

        // 3. TRIGGER CALLBACKS (Event Driven)
        // We iterate through all actions to see if this specific event triggers them.
        for (const auto& [actionID, bindings] : m_actionBindings)
        {
            // Check if this Action is mapped to the Key in the current event
            auto keyIt = m_keyMap.find(actionID);
            if (keyIt != m_keyMap.end() && event.type == InputEvent::Type::Key)
            {
                if (keyIt->second == event.key) // It's a match!
                {
                    // Fire all bindings that match the state (Pressed/Released)
                    for (const auto& binding : bindings)
                    {
                        if (binding.triggerState == (InputActionState)event.state)
                        {
                            binding.callback();
                        }
                    }
                }
            }

            // Check if this Action is mapped to the Mouse Button
            auto mouseIt = m_mouseMap.find(actionID);
            if (mouseIt != m_mouseMap.end() && event.type == InputEvent::Type::MouseButton)
            {
                if (mouseIt->second == event.mouseBtn)
                {
                    for (const auto& binding : bindings)
                    {
                        if (binding.triggerState == (InputActionState)event.state)
                        {
                            binding.callback();
                        }
                    }
                }
            }
        }
    }

    // Clear the local buffer for next frame
    m_frameEvents.clear();
}
