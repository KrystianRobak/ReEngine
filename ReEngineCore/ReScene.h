#pragma once

#include "ReTypes.h"

class Camera;

class ReScene {
public:
    // Constructor always takes a main camera (usually the Editor Camera)
    ReScene(const std::string& name, Camera* camera, bool ownsCamera = true)
        : name_(name), defaultCamera(camera), ownsCamera_(ownsCamera) {
    }

    ~ReScene() {
        // Only delete the default camera if we own it (Editor Camera)
        // We NEVER delete the overrideCamera because it belongs to the ECS
        if (ownsCamera_ && defaultCamera) {
            delete defaultCamera;
            defaultCamera = nullptr;
        }
    }

    // --- Camera Management ---

    // Called by Renderer. Returns Game Camera if playing, otherwise Editor Camera.
    Camera* GetActiveCamera() const {
        if (overrideCamera) return overrideCamera;
        return defaultCamera;
    }

    // Helper for legacy code if needed, but GetActiveCamera is preferred
    Camera* GetDefaultCamera() const { return GetActiveCamera(); }

    // Sets the camera used for Editing (Owned by Scene)
    void SetEditorCamera(Camera* newCamera) {
        if (ownsCamera_ && defaultCamera) delete defaultCamera;
        defaultCamera = newCamera;
        ownsCamera_ = true;
    }

    // Sets the camera used for Gameplay (Owned by ECS/Entity)
    // Pass nullptr to return to Editor Camera.
    void SetOverrideCamera(Camera* gameCamera) {
        overrideCamera = gameCamera;
    }

    void DetachCamera() { ownsCamera_ = false; }

private:
    std::string name_;

    Camera* defaultCamera = nullptr; // The "Editor" Camera (Persistent)
    bool ownsCamera_ = true;

    Camera* overrideCamera = nullptr; // The "Game" Camera (Entity, Temporary)
};