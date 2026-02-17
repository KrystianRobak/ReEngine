// Auto-generated reflection file for ReCamera.h
#include "ReCamera.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> Camera_Variables;
struct Camera_AutoRegister {
    Camera_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "Camera";
        ci.fullName = "Camera";
        ci.module = "ReEngine";
        ci.size = sizeof(Camera);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new Camera(); };
        ci.destruct = [](void* p) { delete static_cast<Camera*>(p); };
        Camera_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("Transform");
        {
            Reflection::ReflectedVariable rv = {
                "CameraTransform", "public",
                false,
                offsetof(Camera, CameraTransform),
                vType
            };
            Camera_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "cameraFront", "public",
                false,
                offsetof(Camera, cameraFront),
                vType,
                "glm, ::, vec3, (, -, 1.0f, -, 1.0f, -, 1.0f, )"
            };
            Camera_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "cameraUp", "public",
                false,
                offsetof(Camera, cameraUp),
                vType,
                "glm, ::, vec3, (, 0.0f, 1.0f, 0.0f, )"
            };
            Camera_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "yaw", "public",
                false,
                offsetof(Camera, yaw),
                vType,
                "-, 90.0f"
            };
            Camera_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "pitch", "public",
                false,
                offsetof(Camera, pitch),
                vType,
                "0.0f"
            };
            Camera_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "lastX", "public",
                false,
                offsetof(Camera, lastX),
                vType,
                "800.0f, /, 2.0"
            };
            Camera_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "lastY", "public",
                false,
                offsetof(Camera, lastY),
                vType,
                "600.0, /, 2.0"
            };
            Camera_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "fov", "public",
                false,
                offsetof(Camera, fov),
                vType,
                "45.0f"
            };
            Camera_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "aspectRatio", "public",
                false,
                offsetof(Camera, aspectRatio),
                vType,
                "800.0f, /, 600.0f"
            };
            Camera_Variables.push_back(std::move(rv));
        }
        ci.variables = Camera_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static Camera_AutoRegister _camera_autoreg;

} // namespace ReflectionGenerated

