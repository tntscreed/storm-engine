#pragma once

#include <memory>

struct SDL_Window;
union SDL_Event;
struct IDirect3DDevice9;

namespace storm::editor
{

class EngineEditor final
{
  public:
    explicit EngineEditor(SDL_Window *window, IDirect3DDevice9 *device);
    ~EngineEditor();

    void StartFrame();
    void EndFrame();

    void ProcessEvent(SDL_Event &event);

    bool IsFocused() const;

  private:
    class Impl;

    std::unique_ptr<Impl> impl_;
};

} // namespace storm::editor
