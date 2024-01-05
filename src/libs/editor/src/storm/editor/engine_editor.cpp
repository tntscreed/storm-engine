#include "storm/editor/engine_editor.hpp"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_sdl2.h>

#ifdef WIN32
#include <Windows.h>
#endif WIN32

namespace storm::editor
{

class EngineEditor::Impl final {
  public:
    ImGuiContext *imgui_;

    bool isFocused_ = true;
    bool showEntityMenu_ = false;
};

EngineEditor::EngineEditor(SDL_Window *window, IDirect3DDevice9 *device)
    : impl_(std::make_unique<Impl>())
{
    IMGUI_CHECKVERSION();
    impl_->imgui_ = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL2_InitForD3D(window);
    ImGui_ImplDX9_Init(device);
}

EngineEditor::~EngineEditor()
{
    ImGui::DestroyContext(impl_->imgui_);
}

void EngineEditor::StartFrame()
{
    const auto &io = ImGui::GetIO();
    const bool is_focused = io.WantCaptureKeyboard;
    if (is_focused != impl_->isFocused_)
    {
#ifdef WIN32
      ShowCursor(is_focused);
#endif
    }
    impl_->isFocused_ =  is_focused;

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    if (ImGui::Begin("Developer Tools", NULL, ImGuiWindowFlags_MenuBar) )
    {
        if (IsFocused() )
        {
            ImGui::Text("Game controls are disabled while using the editor");
        }

        if (ImGui::BeginMenuBar() )
        {
            if (ImGui::BeginMenu("Engine") )
            {
                ImGui::MenuItem("Entities", NULL, &impl_->showEntityMenu_);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (impl_->showEntityMenu_) {
            if (ImGui::Begin("Entities", &impl_->showEntityMenu_, 0) )
            {
                ImGui::Text("There are currently 6 entities");
                ImGui::End();
            }
        }

        ImGui::End();
    }
}

void EngineEditor::EndFrame()
{
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void EngineEditor::ProcessEvent(SDL_Event &event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

bool EngineEditor::IsFocused() const
{
    return impl_->isFocused_;
}

} // namespace storm::editor
