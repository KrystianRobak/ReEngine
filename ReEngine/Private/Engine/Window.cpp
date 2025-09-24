#include "Engine/Window.h"


bool Window::Init(int width, int height, const std::string& title, Editor::IEngineEditorApi* EngineApi)
{
    this->width = width;
    this->height = height;
    this->title = title;

    EngineApi_ = EngineApi;

    RenderCtx->init(this);

    UICtx->init(this);
    
    frameBuffer = new OpenGlFrameBuffer();

    frameBuffer->create_buffers(800, 600);

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

    Render();
}

void Window::on_close()
{
    IsRunning = false;
}

void Window::Render()
{
    for(UIComponent* component : UIComponents)
    {
        component->SetTextureID(frameBuffer->get_texture());
        component->Render();
	}
}

void Window::PreRender()
{
    RenderCtx->pre_render();

    UICtx->pre_render();

    frameBuffer->bind();
}

void Window::PostRender()
{
    frameBuffer->unbind();

    UICtx->post_render();

    RenderCtx->post_render();
}

void Window::InitUiComponets(AssetManagerApi* AssetManager)
{

    for (const auto& component : UIComponents)
    {
        component->Init(EngineApi_ ,AssetManager ,ImGui::GetCurrentContext());
    }
}
