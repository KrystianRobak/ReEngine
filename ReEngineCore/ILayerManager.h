#pragma once

#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Api/IApplicationApi.h"
#include "ILayer.h"
#include "ReTypes.h"
#include "Logger.h"
#include <type_traits>
#include <vector>
#include <memory>
#include <algorithm>
#include <typeinfo>
#include <mutex>
#include <functional>

class ILayerManager
{
public:
    void Init(Editor::IEngineEditorApi* engineAPI, IApplicationApi* EngineApp, ImGuiContext* imguiContext)
    {
        EngineApi_ = engineAPI;
		EngineApp_ = EngineApp;
		context = imguiContext;

		ImGui::SetCurrentContext(context);

		std::string_view temp = Events::Engine::LayerManager::INITIALIZED;

        EngineApi_->SendEvent(temp);
    }

    template<typename LayerTemplate>
    void AddLayerThreadSafe()
    {
        std::lock_guard lock(pendingMutex);
        pendingOps.push_back([this] { AddLayer<LayerTemplate>(); });
    }

    template<typename LayerTemplate>
    LayerTemplate* GetLayer()
    {
        std::lock_guard lock(Mutex_);
        auto it = std::find_if(LayerStack_.begin(), LayerStack_.end(),
            [](auto& layer) { return dynamic_cast<LayerTemplate*>(layer.get()) != nullptr; });

        if (it != LayerStack_.end())
            return static_cast<LayerTemplate*>(it->get());

        return nullptr;
    }

    template<typename LayerTemplate>
        requires(std::is_base_of_v<ILayer, LayerTemplate>)
    void AddLayer()
    {
        std::unique_ptr<LayerTemplate> newLayer;

        {
            std::lock_guard lock(Mutex_);
            auto it = std::find_if(LayerStack_.begin(), LayerStack_.end(),
                [](auto& layer) { return dynamic_cast<LayerTemplate*>(layer.get()) != nullptr; });
            if (it != LayerStack_.end())
                return; // already exists
        }

        // initialize outside lock
        newLayer = std::make_unique<LayerTemplate>();
		newLayer->InitImGuiContext(context);
        newLayer->InitEngineApi(EngineApi_, EngineApp_);
        newLayer->OnAttach();

        {
            std::unique_lock lock(Mutex_);
            LayerStack_.push_back(std::move(newLayer));
        }

    }

    void ProcessPendingOps()
    {
        std::vector<std::function<void()>> ops;
        {
            std::lock_guard lock(pendingMutex);
            std::swap(ops, pendingOps);
        }

        for (auto& op : ops)
            op();
    }

    template<typename LayerTemplate>
        requires(std::is_base_of_v<ILayer, LayerTemplate>)
    void RemoveLayer()
    {
        std::unique_lock lock(Mutex_);

        auto it = std::find_if(LayerStack_.begin(), LayerStack_.end(),
        [](auto& layer) { return dynamic_cast<LayerTemplate*>(layer.get()) != nullptr; });
        
        if (it != LayerStack_.end())
        {
            (*it)->OnDetach();
            LayerStack_.erase(it);
        }
        else
        {
            LOGF_WARN("Layer of type %s does not exist in the stack.", typeid(LayerTemplate).name());
        }
    }

    void OnUpdate()
    {
        std::unique_lock lock(Mutex_);

        for (auto& layer : LayerStack_)
        {
            layer->OnUpdate(0.0f);
        }
    }

private:
    std::vector<std::unique_ptr<ILayer>> LayerStack_;
    Editor::IEngineEditorApi* EngineApi_ = nullptr;
	IApplicationApi* EngineApp_ = nullptr;

    std::vector<std::function<void()>> pendingOps;
    std::mutex pendingMutex;

	ImGuiContext* context = nullptr;

    std::mutex Mutex_; 
};
