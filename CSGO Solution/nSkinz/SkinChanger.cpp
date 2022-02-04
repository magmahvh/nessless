// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <algorithm>
#include <fstream>
#include "SkinChanger.h"
#include "../Settings.hpp"

item_setting* get_by_definition_index(const int definition_index)
{
	for ( int i = 0; i < 36; i++ )
	{
		if ( g_Settings->skins.skinChanger[ i ].itemId == definition_index || g_Settings->skins.skinChanger[ i ].itemIdIndex == definition_index )
		{

			return &g_Settings->skins.skinChanger[ i ];
		}
	}

	return nullptr;
}

bool bModelBackup = false;

static const char* player_model_index_t[] =
{
	"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
	"models/player/custom_player/legacy/tm_phoenix_varianth.mdl",
	"models/player/custom_player/legacy/tm_leet_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
	"models/player/custom_player/legacy/tm_leet_varianth.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
	"models/player/custom_player/legacy/tm_leet_varianti.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
	"models/player/custom_player/legacy/tm_leet_variantf.mdl"
};

static const char* player_model_index_ct[] =
{
	"models/player/custom_player/legacy/ctm_st6_variante.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
	"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
	"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantb.mdl"
};


Memory memory;

std::unordered_map <std::string, int> SkinChanger::model_indexes;
std::unordered_map <std::string, int> SkinChanger::player_model_indexes;

std::vector <SkinChanger::PaintKit> SkinChanger::skinKits;
std::vector <SkinChanger::PaintKit> SkinChanger::gloveKits;
std::vector <SkinChanger::PaintKit> SkinChanger::displayKits;

static std::unordered_map <std::string_view, const char*> iconOverrides;

void erase_override_if_exists_by_index(const int definition_index) noexcept
{
    if (auto original_item = game_data::get_weapon_info(definition_index)) 
	{
        if (!original_item->icon)
            return;

        if (const auto override_entry = iconOverrides.find(original_item->icon); override_entry != end(iconOverrides))
            iconOverrides.erase(override_entry);
    }
}

void apply_config_on_attributable_item(C_BaseAttributableItem* item, const item_setting* config, const unsigned xuid_low) noexcept
{
    item->m_iItemIDHigh() = -1; //-V522
    item->m_iAccountID() = xuid_low;
	item->m_flFallbackWear() = config->wear;

	volatile int nQuality = config->quality;
    if ( nQuality )
        item->m_iEntityQuality() = nQuality;

	volatile int nPaintKit = config->paintKit;
    if ( nPaintKit )
        item->m_nFallbackPaintKit() = nPaintKit;

	volatile int nSeed = config->seed;
    if ( nSeed )
        item->m_nFallbackSeed() = nSeed;

	volatile int nStatTrak = config->stat_trak;
    if ( nStatTrak )
        item->m_nFallbackStatTrak() = nStatTrak;

    auto& definition_index = *( int16_t* )( ( DWORD )( item ) + 0x2FAA );

    if (config->definition_override_index && config->definition_override_index != definition_index)
    {
        if (auto replacement_item = game_data::get_weapon_info(config->definition_override_index)) 
		{
            auto old_definition_index = definition_index;
            definition_index = config->definition_override_index;

			if (SkinChanger::model_indexes.find(replacement_item->model) == SkinChanger::model_indexes.end())
				SkinChanger::model_indexes.emplace(replacement_item->model, g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex(replacement_item->model));

            item->SetModelIndex(SkinChanger::model_indexes.at(replacement_item->model));
            item->PreDataUpdate(0);

            if (old_definition_index) 
                if (auto original_item = game_data::get_weapon_info(old_definition_index); original_item && original_item->icon && replacement_item->icon)
                    iconOverrides[original_item->icon] = replacement_item->icon;
        }
    } 
	else
        erase_override_if_exists_by_index(definition_index);
}

static auto get_wearable_create_fn() -> CreateClientClassFn
{
	auto classes = g_Globals.m_Interfaces.m_CHLClient->GetAllClasses( );

	while (classes->m_ClassID != ClassId_CEconWearable)
		classes = classes->m_pNext;

	return classes->m_pCreateFn;
}

static C_BaseAttributableItem* make_glove(int entry, int serial) noexcept
{
	get_wearable_create_fn()(entry, serial);
	auto glove = static_cast <C_BaseAttributableItem*> (g_Globals.m_Interfaces.m_EntityList->GetClientEntity(entry));

	if (!glove)
		return nullptr;

	glove->SetAbsoluteOrigin( Vector(16384.0f, 16384.0f, 16384.0f));
	return glove;
}

static float last_skins_update = 0.0f;

static void post_data_update_start(C_BasePlayer* local) noexcept
{
    C_PlayerInfo player_info;

    if (!g_Globals.m_Interfaces.m_EngineClient->GetPlayerInfo(local->EntIndex(), &player_info))
        return;

	static auto glove_handle = CBaseHandle(0);

	auto wearables = local->m_hMyWearables();
	auto glove_config = get_by_definition_index(GLOVE_T_SIDE);
	auto glove = reinterpret_cast <C_BaseAttributableItem*> (g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(wearables[0]));

	if (!glove)
	{
		auto our_glove = reinterpret_cast <C_BaseAttributableItem*> (g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(glove_handle));

		if (our_glove)
		{
			wearables[0] = glove_handle;
			glove = our_glove;
		}
	}

	if (!local->IsAlive())
	{
		if (glove)
		{
			glove->GetClientNetworkable()->SetDestroyedOnRecreateEntities();
			glove->GetClientNetworkable()->Release();
		}

		return;
	}

	if (glove_config && glove_config->definition_override_index)
	{
		if (!glove)
		{
			auto entry = g_Globals.m_Interfaces.m_EntityList->GetHighestEntityIndex() + 1;
			auto serial = rand() % 0x1000;

			glove = make_glove(entry, serial);
			wearables[0] = entry | serial << 16;
			glove_handle = wearables[0];
		}

		*reinterpret_cast <int*> (uintptr_t(glove) + 0x64) = -1;
		apply_config_on_attributable_item(glove, glove_config, player_info.m_iXuidLow);
	}
	
	auto weapons = local->m_hMyWeapons();

	for (auto weapon_handle = 0; weapons[weapon_handle].IsValid(); weapon_handle++) 
	{
		auto weapon = (C_BaseCombatWeapon*)g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(weapons[weapon_handle]); //-V807
		if (!weapon) 
			continue;

		auto definition_index = weapon->m_iItemDefinitionIndex();
		auto active_conf = get_by_definition_index(weapon->IsKnife() ? WEAPON_KNIFE : definition_index);
		if ( active_conf )
			apply_config_on_attributable_item(weapon, active_conf, player_info.m_iXuidLow);
		else
			erase_override_if_exists_by_index(definition_index);
	}

	auto view_model = (C_BaseViewModel*)g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(local->m_hViewModel());

    if (!view_model)
        return;

    auto view_model_weapon = (C_BaseCombatWeapon*)g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(view_model->m_hWeapon());

    if (!view_model_weapon)
        return;

    auto override_info = game_data::get_weapon_info(view_model_weapon->m_iItemDefinitionIndex());

    if (!override_info)
        return;

    auto world_model = (C_BaseCombatWeapon*)g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(view_model_weapon->m_hWeaponWorldModel());

    if (!world_model)
        return;

	if (SkinChanger::model_indexes.find(override_info->model) == SkinChanger::model_indexes.end())
		SkinChanger::model_indexes.emplace(override_info->model, g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex(override_info->model));

	view_model->m_nEntityModelIndex() = SkinChanger::model_indexes.at(override_info->model);
    world_model->m_nEntityModelIndex() = SkinChanger::model_indexes.at(override_info->model) + 1;
}

static bool UpdateRequired = false;
static bool hudUpdateRequired = false;

static void updateHud() noexcept
{
    if (auto hud_weapons = memory.findHudElement(memory.hud, _S("CCSGO_HudWeaponSelection")) - 0x28)
        for (auto i = 0; i < *(hud_weapons + 0x20); i++)
            i = memory.clearHudWeapon(hud_weapons, i);
    
    hudUpdateRequired = false;
}

void SkinChanger::run(ClientFrameStage_t stage) noexcept
{
	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	if (!g_Globals.m_LocalPlayer)
		return;

	post_data_update_start(g_Globals.m_LocalPlayer);

	if (!g_Globals.m_LocalPlayer->IsAlive()) //-V807
	{
		UpdateRequired = false;
		hudUpdateRequired = false;
		return;
	}

	static auto backup_model_index = -1;
	{
		const char** player_model_index = nullptr;
		auto player_model = 0;

		switch (g_Globals.m_LocalPlayer->m_iTeamNum())
		{
		case 2:
			player_model_index = player_model_index_t;
			player_model = g_Settings->m_nModelT;
			break;
		case 3:
			player_model_index = player_model_index_ct;
			player_model = g_Settings->m_nModelCT;
			break;
		}

		if (player_model)
		{
			if (!bModelBackup)
			{
				auto model = g_Globals.m_LocalPlayer->GetModel();

				if (model)
				{
					auto studio_model = g_Globals.m_Interfaces.m_ModelInfo->GetStudiomodel(model);

					if (studio_model)
					{
						auto name = _S("models/") + (std::string)studio_model->szName;
						backup_model_index = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex(name.c_str());
					}
				}
			}

			if (SkinChanger::player_model_indexes.find(player_model_index[player_model - 1]) == SkinChanger::player_model_indexes.end()) //-V522
				SkinChanger::player_model_indexes.emplace(player_model_index[player_model - 1], g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex(player_model_index[player_model - 1]));

			g_Globals.m_LocalPlayer->SetModelIndex(SkinChanger::player_model_indexes[player_model_index[player_model - 1]]);
			bModelBackup = true;
		}
		else if (bModelBackup)
		{
			g_Globals.m_LocalPlayer->SetModelIndex(backup_model_index);
			bModelBackup = false;
		}
	}

	if (UpdateRequired)
	{
		UpdateRequired = false;
		hudUpdateRequired = true;

		g_Globals.m_Interfaces.m_ClientState->m_nDeltaTick( ) = -1;
	}
	else if ( hudUpdateRequired )
	{
		hudUpdateRequired = false;
		updateHud();
	}
}

void SkinChanger::scheduleHudUpdate() noexcept
{
	//if (!g_Globals.m_LocalPlayer->IsAlive())
		//return;

	if (!g_Globals.m_LocalPlayer)
		return;

	if ( g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime - last_skins_update < 1.0f)
		return;

	UpdateRequired = true;
	last_skins_update = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
}

void SkinChanger::overrideHudIcon(C_GameEvent* event) noexcept
{
	if (auto iconOverride = iconOverrides[event->GetString(_S("weapon"))])
		event->SetString(_S("weapon"), iconOverride);
}