#include "storm\blade\blade.hpp"

#include "animation.h"
#include "core.h"
#include "geometry.h"
#include "shared/messages.h"
#include "string_compare.hpp"

#include <storm/editor/storm_imgui.hpp>

CREATE_CLASS(BLADE)

static const char *handName = "saber_hand";
static const char *beltName = "saber_belt";
static const char *bladeStart = "start";
static const char *bladeEnd = "end";

static const char *gunHandName = "gun_hand";
static const char *gunBeltName = "gun_belt";
static const char *gunFire = "gun_fire";

static const char *sabergunHandName = "saberGun_hand";
static const char *sabergunBeltName = "saberGun_belt";

BLADE::BLADE_INFO::BLADE_INFO()
    : id_("saber")
    , parentEntityId_(0)
    , vrt{}
{
    locatorName_ = beltName;
    defLifeTime = 0.15f;
    lifeTime = 0.0f;
    color[0] = 0x80162EBE;
    color[1] = 0x00FF0000;
    time = 0.0f;
    for (int32_t v = 0; v < WAY_LENGTH; v++)
        vrtTime[v] = -1e20f;
}

BLADE::BLADE_INFO::~BLADE_INFO()
{
    core.EraseEntity(parentEntityId_);
}

void BLADE::BLADE_INFO::DrawBlade(VDX9RENDER *rs, unsigned int blendValue, MODEL *mdl, NODE *manNode)
{
    auto *obj = static_cast<MODEL *>(core.GetEntityPointer(parentEntityId_));
    if (obj == nullptr)
    {
        return;
    }

    auto *bladeNode = obj->GetNode(0);
    if (bladeNode == nullptr)
    {
        return;
    }

    CMatrix perMtx;
    if ((blendValue & 0xff000000) == 0xff000000)
    {
        bladeNode->SetTechnique("EnvAmmoShader");
    }
    else
    {
        bladeNode->SetTechnique("AnimationBlend");
    }
    int32_t sti = -1;
    const std::string &locator_name = active_ ? locatorNameActive_ : locatorName_;
        auto idBlade = manNode->geo->FindName(locator_name.c_str() );

    if ((sti = manNode->geo->FindLabelN(sti + 1, idBlade)) > -1)
    {
        auto *ani = mdl->GetAnimation();
        auto *bones = &ani->GetAnimationMatrix(0);

        GEOS::LABEL lb;
        manNode->geo->GetLabel(sti, lb);
        CMatrix mt;
        mt.Vx() = CVECTOR(lb.m[0][0], lb.m[0][1], lb.m[0][2]);
        mt.Vy() = CVECTOR(lb.m[1][0], lb.m[1][1], lb.m[1][2]);
        mt.Vz() = CVECTOR(lb.m[2][0], lb.m[2][1], lb.m[2][2]);
        mt.Pos() = CVECTOR(lb.m[3][0], lb.m[3][1], lb.m[3][2]);

        auto mbn = mt * bones[lb.bones[0]];
#ifndef _WIN32 // FIX_LINUX DirectXMath
        mbn.Pos().x *= -1.0f;
        mbn.Vx().x *= -1.0f;
        mbn.Vy().x *= -1.0f;
        mbn.Vz().x *= -1.0f;
#endif
        CMatrix scl;
        scl.Vx().x = -1.0f;
        scl.Vy().y = 1.0f;
        scl.Vz().z = 1.0f;
        mbn.EqMultiply(scl, CMatrix(mbn));

        perMtx = mbn * mdl->mtx;
    }
    obj->mtx = perMtx;
    obj->ProcessStage(Stage::realize, 0);

    //--------------------------------------------------------------------------
    rs->SetTexture(0, nullptr);
    rs->SetTransform(D3DTS_WORLD, CMatrix());

    // move to the beginning
    int32_t first = 0; // end vertex 2 draw
    for (int32_t v = WAY_LENGTH - 2; v > -1; v--)
    {
        vrt[v * 2 + 2] = vrt[v * 2 + 0];
        vrt[v * 2 + 3] = vrt[v * 2 + 1];
        vrtTime[v + 1] = vrtTime[v + 0];

        if (vrtTime[v + 1] + lifeTime <= time)
            continue;

        first++;

        auto blend = (time - vrtTime[v + 1]) / lifeTime;

        auto fcol0 = static_cast<float>((color[0] >> 24) & 0xFF);
        auto fcol1 = static_cast<float>((color[1] >> 24) & 0xFF);
        auto a = static_cast<uint32_t>(fcol0 + blend * (fcol1 - fcol0));

        fcol0 = static_cast<float>((color[0] >> 16) & 0xFF);
        fcol1 = static_cast<float>((color[1] >> 16) & 0xFF);
        auto r = static_cast<uint32_t>(fcol0 + blend * (fcol1 - fcol0));

        fcol0 = static_cast<float>((color[0] >> 8) & 0xFF);
        fcol1 = static_cast<float>((color[1] >> 8) & 0xFF);
        auto g = static_cast<uint32_t>(fcol0 + blend * (fcol1 - fcol0));

        fcol0 = static_cast<float>((color[0] >> 0) & 0xFF);
        fcol1 = static_cast<float>((color[1] >> 0) & 0xFF);
        auto b = static_cast<uint32_t>(fcol0 + blend * (fcol1 - fcol0));

        vrt[v * 2 + 2].diffuse = vrt[v * 2 + 3].diffuse = (a << 24) | (r << 16) | (g << 8) | b;
    }

    // search for start
    sti = bladeNode->geo->FindLabelN(0, bladeNode->geo->FindName(bladeStart));
    GEOS::LABEL lb;
    if (sti >= 0)
    {
        bladeNode->geo->GetLabel(sti, lb);
        vrt[0].pos = perMtx * CVECTOR(lb.m[3][0], lb.m[3][1], lb.m[3][2]);
        vrt[0].diffuse = color[0];
        sti = bladeNode->geo->FindLabelN(0, bladeNode->geo->FindName(bladeEnd));
        if (sti >= 0)
        {
            bladeNode->geo->GetLabel(sti, lb);
            vrt[1].pos = perMtx * CVECTOR(lb.m[3][0], lb.m[3][1], lb.m[3][2]);
            vrt[1].diffuse = color[0];
            vrtTime[0] = time;

            auto bDraw = rs->TechniqueExecuteStart("Blade");
            if (bDraw)
            {
                if (first > 0)
                    rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, FVF, first * 2, &vrt[0], sizeof vrt[0]);
            }
            while (rs->TechniqueExecuteNext())
            {
            }
        }
        else
        {
            core.Trace(R"(BLADE::Realize -> no find locator "%s", model "%s")", bladeEnd, bladeNode->GetName());
        }
    }
    else
    {
        core.Trace(R"(BLADE::Realize -> no find locator "%s", model "%s")", bladeStart, bladeNode->GetName());
    }
}

bool BLADE::BLADE_INFO::LoadBladeModel(MESSAGE &message)
{
    core.EraseEntity(parentEntityId_);

    // model name
    const std::string &mdlName = message.String();
    if (!mdlName.empty())
    {
        // path of the model
        char path[256];
        strcpy_s(path, "Ammo\\");
        strcat_s(path, mdlName.c_str());
        // path of the textures
        auto *gs = static_cast<VGEOMETRY *>(core.GetService("geometry"));
        if (gs)
            gs->SetTexturePath("Ammo\\");
        // Create a model
        parentEntityId_ = core.CreateEntity("modelr");
        if (!core.Send_Message(parentEntityId_, "ls", MSG_MODEL_LOAD_GEO, path))
        {
            core.EraseEntity(parentEntityId_);
            if (gs)
                gs->SetTexturePath("");
            return false;
        }
        if (gs)
            gs->SetTexturePath("");
        // trace parameters
        defLifeTime = message.Float();
        color[0] = message.Long();
        color[1] = message.Long();
    }
    else
        return false;
    return true;
}

BLADE::BLADE()
{
    gunLocator_ = gunBeltName;
    gunLocatorActive_ = gunHandName;
    blendValue = 0xFFFFFFFF;

    blades_[0].locatorName_ = beltName;
    blades_[0].locatorNameActive_ = handName;
    blades_[0].id_ = "saber";
    blades_[1].locatorName_ = sabergunBeltName;
    blades_[1].locatorNameActive_ = sabergunHandName;
    blades_[1].id_ = "saberGun";
}

BLADE::~BLADE()
{
    core.EraseEntity(gun);

    for (int32_t i = 0; i < ITEMS_INFO_QUANTITY; i++)
        items[i].Release();
}

bool BLADE::Init()
{
    // GUARD(BLADE::BLADE())

    col = static_cast<COLLIDE *>(core.GetService("coll"));
    if (col == nullptr)
        throw std::runtime_error("No service: COLLIDE");

    core.AddToLayer(REALIZE, GetId(), 65550);

    rs = static_cast<VDX9RENDER *>(core.GetService("dx9render"));
    if (!rs)
        throw std::runtime_error("No service: dx9render");

    // UNGUARD
    return true;
}

//------------------------------------------------------------------------------------
// realize
//------------------------------------------------------------------------------------
void BLADE::Realize(uint32_t Delta_Time)
{
    blades_[0].time += 0.001f * (Delta_Time);

    auto *mdl = static_cast<MODEL *>(core.GetEntityPointer(man));
    if (!mdl)
        return;

    auto *manNode = mdl->GetNode(0);
    rs->TextureSet(0, -1);
    rs->TextureSet(1, -1);
    rs->TextureSet(2, -1);
    rs->TextureSet(3, -1);

    CMatrix mtx;
    rs->GetTransform(D3DTS_VIEW, mtx);
    mtx.Transposition();
    mtx.Pos() = 0.0f;
    rs->SetTransform(D3DTS_TEXTURE1, mtx);
    rs->SetRenderState(D3DRS_TEXTUREFACTOR, blendValue);

    //------------------------------------------------------
    // draw gun
    CMatrix perMtx;
    int32_t sti;
    auto *obj = static_cast<MODEL *>(core.GetEntityPointer(gun));
    if (obj != nullptr)
    {
        auto *gunNode = obj->GetNode(0);
        if (gunNode != nullptr)
        {
            if ((blendValue & 0xff000000) == 0xff000000)
            {
                gunNode->SetTechnique("EnvAmmoShader");
            }
            else
            {
                gunNode->SetTechnique("AnimationBlend");
            }
        }
        sti = -1;
        const std::string &gun_locator = isGunActive_ ? gunLocatorActive_ : gunLocator_;
        auto idGun = manNode->geo->FindName(gun_locator.c_str());

        if ((sti = manNode->geo->FindLabelN(sti + 1, idGun)) > -1)
        {
            auto *ani = mdl->GetAnimation();
            auto *bones = &ani->GetAnimationMatrix(0);

            GEOS::LABEL lb;
            manNode->geo->GetLabel(sti, lb);
            CMatrix mt;
            mt.Vx() = CVECTOR(lb.m[0][0], lb.m[0][1], lb.m[0][2]);
            mt.Vy() = CVECTOR(lb.m[1][0], lb.m[1][1], lb.m[1][2]);
            mt.Vz() = CVECTOR(lb.m[2][0], lb.m[2][1], lb.m[2][2]);
            mt.Pos() = CVECTOR(lb.m[3][0], lb.m[3][1], lb.m[3][2]);

            auto mbn = mt * bones[lb.bones[0]];
#ifndef _WIN32 // FIX_LINUX DirectXMath
            mbn.Pos().x *= -1.0f;
            mbn.Vx().x *= -1.0f;
            mbn.Vy().x *= -1.0f;
            mbn.Vz().x *= -1.0f;
#endif
            CMatrix scl;
            scl.Vx().x = -1.0f;
            scl.Vy().y = 1.0f;
            scl.Vz().z = 1.0f;
            mbn.EqMultiply(scl, CMatrix(mbn));

            perMtx = mbn * mdl->mtx;
        }
        obj->mtx = perMtx;

        obj->ProcessStage(Stage::realize, 0);
    }

    //------------------------------------------------------
    // draw saber
    blades_[0].DrawBlade(rs, blendValue, mdl, manNode);
    blades_[1].DrawBlade(rs, blendValue, mdl, manNode);

    //------------------------------------------------------
    // draw tied items
    for (int32_t n = 0; n < ITEMS_INFO_QUANTITY; n++)
    {
        if (items[n].nItemIndex != -1)
        {
            items[n].DrawItem(rs, blendValue, mdl, manNode);
        }
    }

    mtx.SetIdentity();
    rs->SetTransform(D3DTS_TEXTURE1, mtx);
}

bool BLADE::LoadBladeModel(MESSAGE &message)
{
    const auto nBladeIdx = message.Long();
    if (nBladeIdx < 0 || nBladeIdx >= blades_.size())
    {
        return false;
    }

    man = message.EntityID();

    if (nBladeIdx == 1)
    {
        blades_[nBladeIdx].locatorName_ = sabergunBeltName;
    }
    else
    {
        blades_[nBladeIdx].locatorName_ = beltName;
    }

    return blades_[nBladeIdx].LoadBladeModel(message);
}

bool BLADE::LoadGunModel(MESSAGE &message)
{
    core.EraseEntity(gun);
    man = message.EntityID();
    // model name
    const std::string &mdlName = message.String();
    if (!mdlName.empty())
    {
        // path of the model
        char path[256];
        strcpy_s(path, "Ammo\\");
        strcat_s(path, mdlName.c_str());
        // path of the textures
        auto *gs = static_cast<VGEOMETRY *>(core.GetService("geometry"));
        if (gs)
            gs->SetTexturePath("Ammo\\");
        // Create a model
        gun = core.CreateEntity("modelr");
        if (!core.Send_Message(gun, "ls", MSG_MODEL_LOAD_GEO, path))
        {
            core.EraseEntity(gun);
            if (gs)
                gs->SetTexturePath("");
            return false;
        }
        if (gs)
            gs->SetTexturePath("");
    }
    else
        return false;
    return true;
}

void BLADE::GunFire()
{
    auto *mdl = static_cast<MODEL *>(core.GetEntityPointer(man));
    auto *manNode = mdl->GetNode(0);

    //------------------------------------------------------
    // search gunfire
    CMatrix perMtx;
    int32_t sti;

    std::string currentGunLocName = isGunActive_ ? gunLocatorActive_ : gunLocator_;

    auto *obj = static_cast<MODEL *>(core.GetEntityPointer(gun));
    if (obj == nullptr) // no pistol - look for saber pistol
    {
        obj = static_cast<MODEL *>(core.GetEntityPointer(blades_[1].parentEntityId_));
        currentGunLocName = blades_[1].locatorName_;
    }

    if (obj != nullptr)
    {
        auto *gunNode = obj->GetNode(0);
        sti = -1;
        auto idGun = manNode->geo->FindName(currentGunLocName.c_str() );

        if ((sti = manNode->geo->FindLabelN(sti + 1, idGun)) > -1)
        {
            auto *ani = mdl->GetAnimation();
            auto *bones = &ani->GetAnimationMatrix(0);

            GEOS::LABEL lb;
            manNode->geo->GetLabel(sti, lb);
            CMatrix mt;
            mt.Vx() = CVECTOR(lb.m[0][0], lb.m[0][1], lb.m[0][2]);
            mt.Vy() = CVECTOR(lb.m[1][0], lb.m[1][1], lb.m[1][2]);
            mt.Vz() = CVECTOR(lb.m[2][0], lb.m[2][1], lb.m[2][2]);
            mt.Pos() = CVECTOR(lb.m[3][0], lb.m[3][1], lb.m[3][2]);

            auto mbn = mt * bones[lb.bones[0]];
#ifndef _WIN32 // FIX_LINUX DirectXMath
            mbn.Pos().x *= -1.0f;
            mbn.Vx().x *= -1.0f;
            mbn.Vy().x *= -1.0f;
            mbn.Vz().x *= -1.0f;
#endif
            perMtx = mbn * mdl->mtx;
        }

        // search for start
        sti = gunNode->geo->FindLabelN(0, gunNode->geo->FindName(gunFire));
        if (sti >= 0)
        {
            GEOS::LABEL lb;
            gunNode->geo->GetLabel(sti, lb);
            CMatrix resm;
            resm.EqMultiply(perMtx, *(CMatrix *)&lb.m);
            auto rp = perMtx * CVECTOR(lb.m[3][0], lb.m[3][1], lb.m[3][2]);

            core.Send_Message(core.GetEntityId("particles"), "lsffffffl", PS_CREATEX, "gunfire", rp.x, rp.y, rp.z,
                              resm.Vz().x, resm.Vz().y, resm.Vz().z, 0);
        }
        else
            core.Trace("MSG_BLADE_GUNFIRE Can't find gun_fire locator");
    }
}

uint64_t BLADE::ProcessMessage(MESSAGE &message)
{
    int32_t n{};

    switch (message.Long())
    {
    case MSG_BLADE_SET: {
        return LoadBladeModel(message);
    }

    case MSG_BLADE_BELT: {
        n = message.Long();
        if (n < blades_.size())
        {
            blades_[n].active_ = false;
            blades_[n].lifeTime = 0.0f;
        }
        break;
    }

    case MSG_BLADE_HAND: {
        n = message.Long();
        if (n < blades_.size())
        {
            blades_[n].active_ = true;
        }
        break;
    }

    case MSG_BLADE_GUNSET: {
        return LoadGunModel(message);
    }
    case MSG_BLADE_GUNBELT: {
        isGunActive_ = false;
        break;
    }
    case MSG_BLADE_GUNHAND: {
        isGunActive_ = true;
        break;
    }
    case MSG_BLADE_GUNFIRE: {
        GunFire();
        break;
    }

    case MSG_BLADE_TRACE_ON: {
        n = message.Long();
        if (n >= 0 && n < blades_.size())
        {
            blades_[n].lifeTime = blades_[n].defLifeTime;
        }
        break;
    }

    case MSG_BLADE_TRACE_OFF: {
        n = message.Long();
        if (n >= 0 && n < blades_.size())
        {
            blades_[n].lifeTime = 0.0f;
        }
        break;
    }

    case MSG_BLADE_ALPHA: {
        blendValue = message.Long();
        break;
    }

    case 1001: {
        man = message.EntityID();
        AddTieItem(message);
        break;
    }
    case 1002: {
        DelTieItem(message);
        break;
    }
    case 1003: {
        DelAllTieItem();
        break;
    }
    }
    return 0;
}

void BLADE::AddTieItem(MESSAGE &message)
{
    const auto nItemIdx = message.Long();

    const std::string &mdlName = message.String();
    const std::string &locName = message.String();

    auto n = FindTieItemByIndex(nItemIdx);
    if (n >= 0)
    {
        core.Trace("Warning! BLADE::AddTieItem(%d,%s,%s) already set that item", nItemIdx, mdlName.c_str(),
                   locName.c_str());
    }
    else
    {
        for (n = 0; n < ITEMS_INFO_QUANTITY; n++)
            if (items[n].nItemIndex == -1)
                break;
        if (n < ITEMS_INFO_QUANTITY)
        {
            items[n].nItemIndex = nItemIdx;
            items[n].LoadItemModel(mdlName.c_str(), locName.c_str());
        }
        else
        {
            core.Trace("Warning! BLADE::AddTieItem(%d,%s,%s) very mach items already set", nItemIdx, mdlName.c_str(),
                       locName.c_str());
        }
    }
}

void BLADE::DelTieItem(MESSAGE &message)
{
    const auto nItemIdx = message.Long();
    const int32_t n = FindTieItemByIndex(nItemIdx);
    if (n >= 0)
        items[n].Release();
}

void BLADE::DelAllTieItem()
{
    for (int32_t i = 0; i < ITEMS_INFO_QUANTITY; i++)
        if (items[i].nItemIndex != -1)
            items[i].Release();
}

int32_t BLADE::FindTieItemByIndex(int32_t n)
{
    if (n < 0)
        return -1;
    for (int32_t i = 0; i < ITEMS_INFO_QUANTITY; i++)
        if (items[i].nItemIndex == n)
            return i;
    return -1;
}

void BLADE::TIEITEM_INFO::Release()
{
    if (nItemIndex != -1)
    {
        nItemIndex = -1;
        core.EraseEntity(eid);
        delete locatorName;
        locatorName = nullptr;
    }
}

void BLADE::TIEITEM_INFO::DrawItem(VDX9RENDER *rs, unsigned int blendValue, MODEL *mdl, NODE *manNode)
{
    auto *obj = static_cast<MODEL *>(core.GetEntityPointer(eid));
    if (obj != nullptr)
    {
        CMatrix perMtx;

        NODE *mdlNode = obj->GetNode(0);
        if ((blendValue & 0xff000000) == 0xff000000)
        {
            mdlNode->SetTechnique("EnvAmmoShader");
        }
        else
        {
            mdlNode->SetTechnique("AnimationBlend");
        }
        int32_t sti = -1;
        auto idLoc = manNode->geo->FindName(locatorName);

        if ((sti = manNode->geo->FindLabelN(sti + 1, idLoc)) > -1)
        {
            Animation *ani = mdl->GetAnimation();
            CMatrix *bones = &ani->GetAnimationMatrix(0);

            GEOS::LABEL lb;
            manNode->geo->GetLabel(sti, lb);
            CMatrix mt;
            mt.Vx() = CVECTOR(lb.m[0][0], lb.m[0][1], lb.m[0][2]);
            mt.Vy() = CVECTOR(lb.m[1][0], lb.m[1][1], lb.m[1][2]);
            mt.Vz() = CVECTOR(lb.m[2][0], lb.m[2][1], lb.m[2][2]);
            mt.Pos() = CVECTOR(lb.m[3][0], lb.m[3][1], lb.m[3][2]);

            CMatrix mbn = mt * bones[lb.bones[0]];
#ifndef _WIN32 // FIX_LINUX DirectXMath
            mbn.Pos().x *= -1.0f;
            mbn.Vx().x *= -1.0f;
            mbn.Vy().x *= -1.0f;
            mbn.Vz().x *= -1.0f;
#endif
            CMatrix scl;
            scl.Vx().x = -1.0f;
            scl.Vy().y = 1.0f;
            scl.Vz().z = 1.0f;
            mbn.EqMultiply(scl, CMatrix(mbn));

            perMtx = mbn * mdl->mtx;
        }
        obj->mtx = perMtx;
        obj->ProcessStage(Stage::realize, 0);
    }
}

bool BLADE::TIEITEM_INFO::LoadItemModel(const char *mdlName, const char *locName)
{
    core.EraseEntity(eid);
    delete locatorName;
    locatorName = nullptr;

    if (!locName || !mdlName)
        return false;

    const auto len = strlen(locName) + 1;
    locatorName = new char[len];
    Assert(locatorName);
    memcpy(locatorName, locName, len);

    // path of the model
    char path[256];
    strcpy_s(path, "Ammo\\");
    strcat_s(path, mdlName);
    // path of the textures
    auto *gs = static_cast<VGEOMETRY *>(core.GetService("geometry"));
    if (gs)
        gs->SetTexturePath("Ammo\\");
    // Create a model
    eid = core.CreateEntity("modelr");
    if (!core.Send_Message(eid, "ls", MSG_MODEL_LOAD_GEO, path))
    {
        core.EraseEntity(eid);
        if (gs)
            gs->SetTexturePath("");
        return false;
    }
    if (gs)
        gs->SetTexturePath("");

    return true;
}

void BLADE::ShowEditor()
{
    ImGui::Text("Blade");
    ImGui::InputScalar("Parent entity", ImGuiDataType_U64, &man);

    size_t i = 0;
    for (auto &blade : blades_)
    {
        ImGui::TextFmt("Blade {}", i++);
        std::string id = fmt::format("Blade {}", i);
        ImGui::PushID(id.c_str());
        ImGui::Checkbox("Equipped", &blade.active_);
        ImGui::InputText("Locator", &blade.locatorName_);
        ImGui::InputText("Active locator", &blade.locatorNameActive_);
        ImGui::PopID();
    }

    ImGui::TextFmt("Gun {}", i++);
    std::string id = fmt::format("Blade {}", i);
    ImGui::PushID(id.c_str());
    ImGui::Checkbox("Equipped", &isGunActive_);
    ImGui::InputText("Locator", &gunLocator_);
    ImGui::InputText("Active locator", &gunLocatorActive_);
    ImGui::PopID();
}

void BLADE::SetEquipmentLocator(const std::string_view &equipment_id, const std::string_view &locator_name)
{
    if (storm::iEquals(equipment_id, "gun") )
    {
        gunLocator_ = locator_name;
    }
    else
    {
        for (auto &blade : blades_)
        {
            if (storm::iEquals(equipment_id, blade.id_) )
            {
                blade.locatorName_ = locator_name;
            }
        }
    }
}
void BLADE::SetEquipmentActiveLocator(const std::string_view &equipment_id, const std::string_view &locator_name)
{
    if (storm::iEquals(equipment_id, "gun") )
    {
        gunLocatorActive_ = locator_name;
    }
    else
    {
        for (auto &blade : blades_)
        {
            if (storm::iEquals(equipment_id, blade.id_) )
            {
                blade.locatorNameActive_ = locator_name;
            }
        }
    }
}