#pragma once
#include "Types.h"
#include "Engine/Core/Coordinator/Coordinator.h"

#define REFFUNCTION(a)
#define REFVARIABLE(a)


#define EXPORT_SCRIPT(ScriptClass)                                \
extern "C" __declspec(dllexport) Script* CreateScriptInstance() { \
    return new ScriptClass();                                     \
}


class Script {
protected:
    Entity entity;
    std::weak_ptr<Coordinator> coordinator;

protected:
    template<typename T>
    T& GetComponent() {
        return coordinator.lock()->GetComponent<T>(entity);
    }

    Entity GetEntity() const {
        return entity;
    }

public:
    virtual ~Script() = default;

    virtual void Init(Entity e, std::shared_ptr<Coordinator> coord) {
        this->entity = e;
        this->coordinator = coord;
    }

    virtual void Begin() = 0;
    virtual void Update() = 0;
    virtual void OnDestroy() = 0;
};
