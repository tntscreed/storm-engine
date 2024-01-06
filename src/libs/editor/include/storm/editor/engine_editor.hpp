#pragma once

#include <memory>

struct SDL_Window;
union SDL_Event;
struct IDirect3DDevice9;

namespace storm::editor
{

enum class DebugFlag
{
    RenderWireframe,
    SoundDebug,
    LocationDebug,
    ExtendedLocationDebug,
    CannonDebug,
};

class EngineEditor final
{
  public:
    explicit EngineEditor(SDL_Window *window, IDirect3DDevice9 *device);
    ~EngineEditor();

    void StartFrame();
    void EndFrame();

    void ProcessEvent(SDL_Event &event);

    bool IsFocused() const;

    bool IsDebugFlagEnabled(DebugFlag flag) const;

  private:
    class Impl;

    std::unique_ptr<Impl> impl_;
};

} // namespace storm::editor
