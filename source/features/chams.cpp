#include "chams.h"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"


Chams::Chams() {
	materialRegular = g_MatSystem->FindMaterial("debug/debugambientcube");
	materialFlat = g_MatSystem->FindMaterial("debug/debugdrawflat");
    materialGlow = g_MatSystem->FindMaterial("dev/glow_armsrace.vmt");
}


void Chams::OverrideMaterial(int type, bool ignoreZ, const Color& rgba) {
	IMaterial* material = nullptr;

	switch (type) {
	case 0:
		material = materialFlat;
		break;
	case 1:
		material = materialRegular;
		break;
    case 2:
        material = materialGlow;
        break;
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);

	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);

    material->AlphaModulate(rgba.a() / 255.0f);

	g_MdlRender->ForcedMaterialOverride(material);
}


void Chams::OnDrawModelExecute(
    IMatRenderContext* ctx,
    const DrawModelState_t& state,
    const ModelRenderInfo_t& info,
    matrix3x4_t* matrix)
{
    static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

    const auto mdl = info.pModel;

    bool is_arm = strstr(mdl->szName, "arms") != nullptr;
    bool is_player = strstr(mdl->szName, "models/player") != nullptr;
    bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
    //bool is_weapon = strstr(mdl->szName, "weapons/v_")  != nullptr;

    if (is_player && g_Options.chams_player_enabled) {
        auto ent = C_BasePlayer::GetPlayerByIndex(info.entity_index);

        if (ent && g_LocalPlayer && ent->IsAlive()) {
            const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
            if (!enemy)
                return;

            const auto clr_front = Color(g_Options.color_chams_player_enemy_visible);
            const auto clr_back = Color(g_Options.color_chams_player_enemy_occluded);

            if (g_Options.chams_player_ignorez) {
                OverrideMaterial(g_Options.chams_player_type, true, clr_back);

                fnDME(g_MdlRender, 0, ctx, state, info, matrix);

                OverrideMaterial(g_Options.chams_player_type, false, clr_front);
            }
            else {
                OverrideMaterial(g_Options.chams_player_type, false, clr_front);
            }
        }
    }
    else if (is_sleeve && g_Options.chams_arms_enabled) {
        auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
        if (!material)
            return;
        //
        // Remove sleeves when drawing Chams.
        //
        material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
        g_MdlRender->ForcedMaterialOverride(material);
    }
    else if (is_arm) {
        auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
        if (!material)
            return;
        else if (g_Options.chams_arms_enabled) {
            OverrideMaterial(g_Options.chams_arm_type, false, Color(g_Options.color_chams_arms));
        }
    }
}