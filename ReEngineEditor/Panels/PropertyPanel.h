#pragma once

#include "UIComponent.h"

class PropertyPanel : public UIComponent
{
public:

    PropertyPanel()
    {
    }

    void ForEachComponent(const char* header, std::vector<const Reflection::ClassInfo*> Components, std::function<void(Entity, const char*)> function);

    void Render() override;


private:
};

