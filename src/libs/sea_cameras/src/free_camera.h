#pragma once

#include "common_camera.h"

class ENTITY_STATE;
class ENTITY_STATE_GEN;
class VDX9RENDER;

class FREE_CAMERA final : public COMMON_CAMERA
{
  public:
    FREE_CAMERA();
    ~FREE_CAMERA() override;

    void SetDevice();
    bool Init() override;
    void Move(uint32_t real_delta);
    void Execute();
    bool CreateState(ENTITY_STATE_GEN *state_gen) const;
    bool LoadState(ENTITY_STATE *state);

    void ProcessStage(Stage stage, uint32_t) override
    {
        switch (stage)
        {
        case Stage::execute:
            Execute();
            break;
        default:
            break;
        }
    }

    void Save(CSaveLoad *pSL) override;
    void Load(CSaveLoad *pSL) override;

  private:
    VDX9RENDER *renderer_ = nullptr;

    CVECTOR position_{};
    CVECTOR angle_{};

    bool cameraOverride_ = false;
};
