#include "Engine/Core/Window.h"

bool Window::Init(int width, int height, const std::string& title)
{
    this->width = width;
    this->height = height;
    this->title = title;

    RenderCtx->init(this);

    UICtx->init(this);

    application->Init();

    propertyPanel = std::make_unique<PropertyPanel>();
    sceneView = std::make_unique<SceneView>(application);
    fileBrowser = std::make_unique<FileBrowser>();
    itemsSelectionPanel = std::make_unique<ItemsSelectionPanel>();
    addingPanel = std::make_unique<AddingPanel>();
    controlPanel = std::make_unique<ControlPanel>();
    animationPanel = std::make_unique<AnimationPanel>();
    keyframeEditor = std::make_unique<KeyframeEditorPanel>();

    std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
    coordinator->AddEventListener(METHOD_LISTENER_ONE_PARAM(Events::Application::MENU_CHANGED, Window::on_mode_Changed));

    return IsRunning;
}

Window::~Window()
{
    UICtx->end();

    RenderCtx->end();
}

void Window::on_mode_Changed(Event& event) {
    MenuType key = static_cast<MenuType>(event.GetParam<int>("MenuType"));
    this->CurrentMode = key;
}

void Window::on_resize(int width, int height)
{
    this->width = width;
    this->height = height;

    sceneView->resize(this->width, this->height);
    Render();
}

void Window::on_close()
{
    IsRunning = false;
}

void Window::Render()
{

    RenderCtx->pre_render();

    UICtx->pre_render();

    application->Render();

    if (application->IsRunning()) {
        application->Update();
    }


    controlPanel->Render();

    itemsSelectionPanel->Render();

    switch(CurrentMode) {
        case MenuType::BaseMenu:

            propertyPanel->Render();

            fileBrowser->Render();
            addingPanel->Render();

            break;

        case MenuType::AnimationMenu:

            animationPanel->Render();
            keyframeEditor->Render();

        break;
    }
    sceneView->Render();





    UICtx->post_render();

    RenderCtx->post_render();

}