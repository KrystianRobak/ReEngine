#pragma once

#include "UIComponent.h"


class PropertyPanel : public UIComponent
{
public:

    PropertyPanel()
    {
    }

    void RenderVariable(const char* varName, const char* typeName, void* varPtr, void* writeData, const char*);

    void OnInit() override;

    void ForEachComponent(const char* header, std::vector<const Reflection::ClassInfo*> Components, std::function<void(Entity, const char*)> function, bool IsAdding);

    void Render() override;


private:
};