#pragma once

#include "ReTypes.h"

class Camera;

class ReScene {
public:
    ReScene(const std::string& name, Camera* camera, bool OwnsCamera = true)
        : name(name), defaultCamera(camera), ownsCamera(OwnsCamera) {
    }

    ~ReScene() {
        if (ownsCamera && defaultCamera) {
            delete defaultCamera;
            defaultCamera = nullptr;
        }
    }

    void SetPath(std::string newPath)
    {
        path = newPath;
    };

    std::string GetPath()
    {
        return path;
    }

    Camera* GetActiveCamera() const {
        if (overrideCamera) return overrideCamera;
        return defaultCamera;
    }

    Camera* GetDefaultCamera() const { return GetActiveCamera(); }

    void SetEditorCamera(Camera* newCamera) {
        if (ownsCamera && defaultCamera) delete defaultCamera;
        defaultCamera = newCamera;
        ownsCamera = true;
    }

    void SetOverrideCamera(Camera* gameCamera) {
        overrideCamera = gameCamera;
    }

    void DetachCamera() { ownsCamera = false; }

private:
    std::string name;
    std::string path;

    Camera* defaultCamera = nullptr;
    bool ownsCamera = true;

    Camera* overrideCamera = nullptr;
};