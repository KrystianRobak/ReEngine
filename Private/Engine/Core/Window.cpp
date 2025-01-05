#include "../Public/Engine/Core/Window.h"

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
   
    return IsRunning;
}

Window::~Window()
{
    UICtx->end();

    RenderCtx->end();
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

    application->Update();

    controlPanel->Render();

    sceneView->Render();

    propertyPanel->Render();

    itemsSelectionPanel->Render();

    fileBrowser->Render();

    addingPanel->Render();

    UICtx->post_render();

    RenderCtx->post_render();

}