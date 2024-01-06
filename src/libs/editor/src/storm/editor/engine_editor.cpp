#include "storm/editor/engine_editor.hpp"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_sdl2.h>

#include <array>

#ifdef WIN32
#include <Windows.h>
#endif WIN32

namespace storm::editor
{

class EngineEditor::Impl final {
  public:
    std::array<bool, 5> debugFlags_;

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
            if (ImGui::BeginMenu("Tools") )
            {
                ImGui::MenuItem("Entities", NULL, &impl_->showEntityMenu_);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Debug") )
            {
                ImGui::MenuItem("Wireframe mode", NULL, &impl_->debugFlags_[static_cast<size_t>(DebugFlag::RenderWireframe)]);
                ImGui::MenuItem("Sound debug", NULL, &impl_->debugFlags_[static_cast<size_t>(DebugFlag::SoundDebug)]);
                ImGui::MenuItem("Location debug", NULL, &impl_->debugFlags_[static_cast<size_t>(DebugFlag::LocationDebug)]);
                ImGui::MenuItem("Extended location debug", NULL, &impl_->debugFlags_[static_cast<size_t>(DebugFlag::ExtendedLocationDebug)]);
                ImGui::MenuItem("Ship cannon debug", NULL, &impl_->debugFlags_[static_cast<size_t>(DebugFlag::CannonDebug)]);
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

bool EngineEditor::IsDebugFlagEnabled(DebugFlag flag) const
{
    return impl_->debugFlags_[static_cast<size_t>(flag)];
}

} // namespace storm::editor
