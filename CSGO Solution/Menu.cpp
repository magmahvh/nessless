#include <d3d9.h>
#include "Tools/Obfuscation/XorStr.hpp"
#include "SDK/Globals.hpp"
#include "Data/Ava.h"

#include "../Features/Model/Model.hpp"
#include <d3dx9.h>
#pragma comment (lib, "d3dx9.lib")

#include "../Render.hpp"
#include "../Config.hpp"
#include "../Settings.hpp"


IDirect3DTexture9* all_skins[36];

std::string get_wep(int id, int custom_index = -1, bool knife = true)
{
	if (custom_index > -1)
	{
		if (knife)
		{
			switch (custom_index)
			{
			case 0: return _S("weapon_knife");
			case 1: return _S("weapon_bayonet");
			case 2: return _S("weapon_knife_css");
			case 3: return _S("weapon_knife_skeleton");
			case 4: return _S("weapon_knife_outdoor");
			case 5: return _S("weapon_knife_cord");
			case 6: return _S("weapon_knife_canis");
			case 7: return _S("weapon_knife_flip");
			case 8: return _S("weapon_knife_gut");
			case 9: return _S("weapon_knife_karambit");
			case 10: return _S("weapon_knife_m9_bayonet");
			case 11: return _S("weapon_knife_tactical");
			case 12: return _S("weapon_knife_falchion");
			case 13: return _S("weapon_knife_survival_bowie");
			case 14: return _S("weapon_knife_butterfly");
			case 15: return _S("weapon_knife_push");
			case 16: return _S("weapon_knife_ursus");
			case 17: return _S("weapon_knife_gypsy_jackknife");
			case 18: return _S("weapon_knife_stiletto");
			case 19: return _S("weapon_knife_widowmaker");
			}
		}
		else
		{
			switch (custom_index)
			{
			case 0: return _S("ct_gloves"); //-V1037
			case 1: return _S("studded_bloodhound_gloves");
			case 2: return _S("t_gloves");
			case 3: return _S("ct_gloves");
			case 4: return _S("sporty_gloves");
			case 5: return _S("slick_gloves");
			case 6: return _S("leather_handwraps");
			case 7: return _S("motorcycle_gloves");
			case 8: return _S("specialist_gloves");
			case 9: return _S("studded_hydra_gloves");
			}
		}
	}
	else
	{
		switch (id)
		{
		case 0: return _S("knife");
		case 1: return _S("gloves");
		case 2: return _S("weapon_ak47");
		case 3: return _S("weapon_aug");
		case 4: return _S("weapon_awp");
		case 5: return _S("weapon_cz75a");
		case 6: return _S("weapon_deagle");
		case 7: return _S("weapon_elite");
		case 8: return _S("weapon_famas");
		case 9: return _S("weapon_fiveseven");
		case 10: return _S("weapon_g3sg1");
		case 11: return _S("weapon_galilar");
		case 12: return _S("weapon_glock");
		case 13: return _S("weapon_m249");
		case 14: return _S("weapon_m4a1_silencer");
		case 15: return _S("weapon_m4a1");
		case 16: return _S("weapon_mac10");
		case 17: return _S("weapon_mag7");
		case 18: return _S("weapon_mp5sd");
		case 19: return _S("weapon_mp7");
		case 20: return _S("weapon_mp9");
		case 21: return _S("weapon_negev");
		case 22: return _S("weapon_nova");
		case 23: return _S("weapon_hkp2000");
		case 24: return _S("weapon_p250");
		case 25: return _S("weapon_p90");
		case 26: return _S("weapon_bizon");
		case 27: return _S("weapon_revolver");
		case 28: return _S("weapon_sawedoff");
		case 29: return _S("weapon_scar20");
		case 30: return _S("weapon_ssg08");
		case 31: return _S("weapon_sg556");
		case 32: return _S("weapon_tec9");
		case 33: return _S("weapon_ump45");
		case 34: return _S("weapon_usp_silencer");
		case 35: return _S("weapon_xm1014");
		default: return _S("unknown");
		}
	}
}

IDirect3DTexture9* get_skin_preview(const char* weapon_name, const std::string& skin_name, IDirect3DDevice9* device)
{
	IDirect3DTexture9* skin_image = nullptr;
	std::string vpk_path;

	if (strcmp(weapon_name, _S("unknown")) && strcmp(weapon_name, _S("knife")) && strcmp(weapon_name, _S("gloves"))) //-V526
	{
		if (skin_name.empty() || skin_name == _S("default"))
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + _S(".png");
		else
			vpk_path = _S("resource/flash/econ/default_generated/") + std::string(weapon_name) + _S("_") + std::string(skin_name) + _S("_light_large.png");
	}
	else
	{
		if (!strcmp(weapon_name, _S("knife")))
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/weapon_knife.png");
		else if (!strcmp(weapon_name, _S("gloves")))
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else if (!strcmp(weapon_name, _S("unknown")))
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/weapon_snowball.png");

	}
	const auto handle = g_Globals.m_Interfaces.m_FileSystem->Open(vpk_path.c_str(), _S("r"), _S("GAME"));
	if (handle)
	{
		int file_len = g_Globals.m_Interfaces.m_FileSystem->Size(handle);
		char* image = new char[file_len]; //-V121

		g_Globals.m_Interfaces.m_FileSystem->Read(image, file_len, handle);
		g_Globals.m_Interfaces.m_FileSystem->Close(handle);

		D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
		delete[] image;
	}

	if (!skin_image)
	{
		std::string vpk_path;

		if (strstr(weapon_name, _S("bloodhound")) != NULL || strstr(weapon_name, _S("hydra")) != NULL)
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + _S(".png");

		const auto handle = g_Globals.m_Interfaces.m_FileSystem->Open(vpk_path.c_str(), _S("r"), _S("GAME"));

		if (handle)
		{
			int file_len = g_Globals.m_Interfaces.m_FileSystem->Size(handle);
			char* image = new char[file_len]; //-V121

			g_Globals.m_Interfaces.m_FileSystem->Read(image, file_len, handle);
			g_Globals.m_Interfaces.m_FileSystem->Close(handle);

			D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
			delete[] image;
		}
	}

	return skin_image;
}

struct tab_struct
{
	float size;
	bool active;
};

bool Tab(const char* label, const ImVec2& size_arg, const bool selected)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
	ImGui::ItemSize(ImVec2(size.x + 10, size.y), style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

	static std::map<ImGuiID, tab_struct> selected_animation;
	auto it_selected = selected_animation.find(ImGui::GetItemID());

	if (it_selected == selected_animation.end())
	{
		selected_animation.insert({ ImGui::GetItemID(), {0.0f, false} });
		it_selected = selected_animation.find(ImGui::GetItemID());
	}
	it_selected->second.size = ImClamp(it_selected->second.size + (5.f * ImGui::GetIO().DeltaTime * (selected || hovered ? 1.f : -1.f)), 0.0f, 1.f);

	ImU32 color_text = ImGui::GetColorU32(ImLerp(ImVec4(83 / 255.f, 83 / 255.f, 84 / 255.f, 1.0f), ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 1.0f), it_selected->second.size));

	window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y + size_arg.y / 3), ImVec2(bb.Max.x, bb.Max.y - size_arg.y / 3), ImGui::GetColorU32(ImVec4(26 / 255.f, 26 / 255.f, 30 / 255.f, 1.0f)), 3);
	window->DrawList->AddRect(ImVec2(bb.Min.x - 1, (bb.Min.y + size_arg.y / 3) - 1), ImVec2(bb.Max.x + 1, (bb.Max.y - size_arg.y / 3) + 1), ImGui::GetColorU32(ImVec4(38 / 255.f, 38 / 255.f, 42 / 255.f, 1.0f)), 3);//outline
	window->DrawList->AddText(ImVec2(bb.Min.x + size_arg.x / 2 - ImGui::CalcTextSize(label).x / 2, bb.Min.y + size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2 - 1), color_text, label);

	return pressed;
}

static float tab_alpha = 0.0f;
static float tab_padding = 0.0f;

void C_Menu::Instance()
{
	if (!m_bIsMenuOpened && ImGui::GetStyle().Alpha > 0.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha - fc / 255.f, 0.f, 1.f);
	}

	if (m_bIsMenuOpened && ImGui::GetStyle().Alpha < 1.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha + fc / 255.f, 0.f, 1.f);
	}

	this->DrawSpectatorList( );
	this->DrawKeybindList( );
	this->WaterMark( );

	if (!m_bIsMenuOpened && ImGui::GetStyle().Alpha < 0.1f)
		return;

	int32_t iScreenSizeX, iScreenSizeY;
	g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(iScreenSizeX, iScreenSizeY);

	ImGui::SetNextWindowPos(ImVec2((iScreenSizeX / 2) - 325, (iScreenSizeY / 2) - 220), ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(797, 450), ImVec2(797, 450));
	ImGui::Begin(this->GetMenuName(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

	auto draw = ImGui::GetWindowDrawList();
	auto pos = ImGui::GetWindowPos();

	static int tab = 0;
	static int subtab = 0;

	static bool draw_anim = false;
	static int anim_tab = 0;

	tab_padding = ImClamp(tab_padding + (75.f * ImGui::GetIO().DeltaTime * (draw_anim && tab != anim_tab ? 1.f : -1.f)), 0.0f, 15.f);
	tab_alpha = ImClamp(tab_alpha + (5.f * ImGui::GetIO().DeltaTime * (draw_anim && tab != anim_tab ? 1.f : -1.f)), 0.0f, 1.f);

	if (tab_alpha >= 1.0f && tab_padding >= 10.0f)
	{
		draw_anim = false;
		tab = anim_tab;
	}

	ImGui::SetCursorPos({ 130,-8 });
	ImGui::BeginGroup();
	{
		if (Tab("Rage", ImVec2(ImGui::CalcTextSize("Rage").x + 15, 60), anim_tab == 0))
			anim_tab = 0, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Anti-aim", ImVec2(ImGui::CalcTextSize("Anti-aim").x + 15, 60), anim_tab == 1))
			anim_tab = 1, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Players", ImVec2(ImGui::CalcTextSize("Players").x + 15, 60), anim_tab == 2))
			anim_tab = 2, draw_anim = true;
		ImGui::SameLine();
		if (Tab("World", ImVec2(ImGui::CalcTextSize("World").x + 15, 60), anim_tab == 3))
			anim_tab = 3, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Misc", ImVec2(ImGui::CalcTextSize("Misc").x + 15, 60), anim_tab == 4))
			anim_tab = 4, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Settings", ImVec2(ImGui::CalcTextSize("Settings").x + 15, 60), anim_tab == 5))
			anim_tab = 5, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Inventory", ImVec2(ImGui::CalcTextSize("Inventory").x + 15, 60), anim_tab == 6))
			anim_tab = 6, draw_anim = true;
	}
	ImGui::EndGroup();

	ImGui::SetCursorPos({ 15,75 + tab_padding });

	ImGui::PushClipRect(ImVec2(pos.x + 1, pos.y + 1), ImVec2(pos.x + 797, pos.y + 450), false);
	ImGui::BeginGroup();
	{
		switch (tab)
		{
		case 0:
			this->DrawRageTab();
			break;
		case 1:
			this->DrawAntiAimTab();
			break;
		case 2:
			this->DrawPlayersTab();
			break;
		case 3:
			this->DrawWorldTab();
			break;
		case 4:
			this->DrawMiscTab();
			break;
		case 5:
			this->DrawConfigTab();
			break;
		case 6:
			this->DrawInventoryTab();
			break;
		}
	}
	ImGui::EndGroup();
	ImGui::PopClipRect();
	ImGui::GetOverlayDrawList()->AddRectFilled({ pos.x + 3,pos.y + 43 }, { pos.x + 797,pos.y + 450 }, ImGui::GetColorU32(ImVec4(22 / 255.0f, 22 / 255.0f, 26 / 255.0f, tab_alpha)), 3);

	ImGui::End();
}

void C_Menu::DrawRageTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;

	ImGui::PopFont();

	//ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild("Main", ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	std::vector< const char* > aRageWeapons =
	{
		( "Auto" ),
		( "Scout" ),
		( "AWP" ),
		( "Deagle" ),
		( "Revolver" ),
		( "Pistol" ),
		( "Rifle" )
	};

	std::vector< const char* > aHitboxes =
	{
		( "Head" ),
		( "Chest" ),
		( "Arms" ),
		( "Pelvis" ),
		( "Stomach" ),
		( "Legs" ),
	};

	static int wep = 0;
	ImGui::SingleSelect( "Weapon", &wep, aRageWeapons );

	ImGui::Checkbox( "Enable Ragebot", &g_Settings->m_aRageSettings[ wep ].m_bEnabled );

	ImGui::Checkbox( "Auto stop", &g_Settings->m_aRageSettings[ wep ].m_bAutoStop );
	ImGui::Checkbox( "Auto scope", &g_Settings->m_aRageSettings[ wep ].m_bAutoScope );

	ImGui::EndChild();

	//ImGui::SetNextWindowPos( ImVec2( iChildPosFirstX, iChildPosSecondY ) );

	ImGui::SameLine();

	ImGui::BeginChild( "Accuracy", ImVec2( iChildDoubleSizeX, iChildDoubleSizeY ) );
	
	ImGui::SliderInt( "Mindamage", &g_Settings->m_aRageSettings[ wep ].m_iMinDamage, 0, 100 );
	ImGui::SliderInt( "Mindamage override", &g_Settings->m_aRageSettings[ wep ].m_iMinDamageOverride, 0, 100 );
	ImGui::SliderInt( "Hitchance", &g_Settings->m_aRageSettings[ wep ].m_iHitChance, 0, 100 );
	ImGui::SliderInt( "DT Hitchance", &g_Settings->m_aRageSettings[ wep ].m_iDoubleTapHitChance, 0, 100 );
	ImGui::SliderInt( "Accuracy boost", &g_Settings->m_aRageSettings[ wep ].m_iAccuracyBoost, 0, 100 );
	ImGui::SliderFloat( "Recharge time", g_Settings->m_flRechargeTime, 0.f, 2.f );

	std::vector < const char* > aAutoStop =
	{
		"Force accuracy",
		"Early"
	};

	if ( ImGui::BeginCombo( _S( "Autostop options" ), _S( "Select" ), 0, aAutoStop.size( ) ) )
	{
		for ( int i = 0; i < aAutoStop.size( ); i++ )
			ImGui::Selectable( aAutoStop[ i ], &g_Settings->m_aRageSettings[ wep ].m_AutoStopOptions[ i ], ImGuiSelectableFlags_DontClosePopups );

		ImGui::EndCombo( );
	}

	std::vector < const char* > aDoubleTap =
	{
		"Move between shots",
		"Full stop"
	};

	if ( ImGui::BeginCombo( _S( "DT options" ), _S( "Select" ), 0, aDoubleTap.size( ) ) )
	{
		for ( int i = 0; i < aDoubleTap.size( ); i++ )
			ImGui::Selectable( aDoubleTap[ i ], &g_Settings->m_aRageSettings[ wep ].m_DoubleTapOptions[ i ], ImGuiSelectableFlags_DontClosePopups );

		ImGui::EndCombo( );
	}

	ImGui::EndChild();
	
	ImGui::SameLine();

	ImGui::BeginChild("Target", ImVec2(iChildDoubleSizeX, 230));

	if (ImGui::BeginCombo(_S("Hitboxes"), _S("Select"), 0, aHitboxes.size()))
	{
		for (int i = 0; i < aHitboxes.size(); i++)
			ImGui::Selectable(aHitboxes[i], &g_Settings->m_aRageSettings[wep].m_Hitboxes[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo(_S("Safe Hitboxes"), _S("Select"), 0, aHitboxes.size()))
	{
		for (int i = 0; i < aHitboxes.size(); i++)
			ImGui::Selectable(aHitboxes[i], &g_Settings->m_aRageSettings[wep].m_SafeHitboxes[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo(_S("Multipoints"), _S("Select"), 0, aHitboxes.size()))
	{
		for (int i = 0; i < aHitboxes.size(); i++)
			ImGui::Selectable(aHitboxes[i], &g_Settings->m_aRageSettings[wep].m_Multipoints[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	ImGui::SliderInt("Head scale", &g_Settings->m_aRageSettings[wep].m_iHeadScale, 0, 100);
	ImGui::SliderInt("Body scale", &g_Settings->m_aRageSettings[wep].m_iBodyScale, 0, 100);

	ImGui::Checkbox(_S("Lethal safe"), &g_Settings->m_aRageSettings[wep].m_bLethalSafety);
	ImGui::Checkbox(_S("Prefer body"), &g_Settings->m_aRageSettings[wep].m_bPreferBody);
	ImGui::Checkbox(_S("Prefer safe"), &g_Settings->m_aRageSettings[wep].m_bPreferSafe);

	ImGui::EndChild();

	//ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosFirstY));
	ImGui::SetCursorPos({ 534,338 + tab_padding });
	ImGui::BeginChild("Advanced", ImVec2(iChildDoubleSizeX, 100));

	ImGui::Text(_S("Force safe"));
	ImGui::Keybind(_S("Force safe"), &g_Settings->m_aSafePoint->m_iKeySelected, &g_Settings->m_aSafePoint->m_iModeSelected);

	ImGui::Text(_S("Damage override"));
	ImGui::Keybind(_S("Damage override"), &g_Settings->m_aMinDamage->m_iKeySelected, &g_Settings->m_aMinDamage->m_iModeSelected);

	ImGui::Text(_S("Double tap"));
	ImGui::Keybind(_S("Double tap"), &g_Settings->m_aDoubleTap->m_iKeySelected, &g_Settings->m_aDoubleTap->m_iModeSelected);

	ImGui::Text(_S("Hide shots"));
	ImGui::Keybind(_S("Hide shots"), &g_Settings->m_aHideShots->m_iKeySelected, &g_Settings->m_aHideShots->m_iModeSelected);

	ImGui::EndChild();

	//ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosSecondY));
}

void C_Menu::DrawAntiAimTab( )
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;

	int iChildSizeX = (800 - ImGui::CalcTextSize(this->GetMenuName()).x - 40) - 30;
	int iChildSizeY = (565 - 30) - 200;

	ImGui::PopFont( );

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 15 + 40;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 40;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;

	//ImGui::SetNextWindowPos( ImVec2( iChildPosFirstX, iChildPosFirstY ) );
	ImGui::BeginChild( _S( "Main" ), ImVec2( iChildDoubleSizeX, iChildDoubleSizeY ) );

	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox( _S( "Enable Anti-aim" ), g_Settings->m_bAntiAim );
	ImGui::Checkbox( _S( "At targets" ), g_Settings->m_bAntiAimAtTargets );
	ImGui::Checkbox( _S( "Auto direction" ), g_Settings->m_bAutoDirection );

	ImGui::SliderInt( _S( "Yaw add offset" ), g_Settings->m_iYawAddOffset, 0, 180 );
	ImGui::SliderInt( _S( "Jitter amount" ), g_Settings->m_iJitterAmount, 0, 180 );

	ImGui::Text( _S( "Manual left" ) );
	ImGui::Keybind( _S( "123" ), &g_Settings->m_aManualLeft->m_iKeySelected, &g_Settings->m_aManualLeft->m_iModeSelected );

	ImGui::Text( _S( "Manual back" ) );
	ImGui::Keybind( _S( "1234" ), &g_Settings->m_aManualBack->m_iKeySelected, &g_Settings->m_aManualBack->m_iModeSelected );

	ImGui::Text( _S( "Manual right" ) );
	ImGui::Keybind( _S( "12346" ), &g_Settings->m_aManualRight->m_iKeySelected, &g_Settings->m_aManualRight->m_iModeSelected );

	ImGui::EndChild( );
	ImGui::SameLine();
	//ImGui::SetNextWindowPos( ImVec2( iChildPosFirstX, iChildPosSecondY - 30 ) );
	ImGui::BeginChild( _S( "Angle settings" ), ImVec2( iChildDoubleSizeX, iChildDoubleSizeY ) );

	ImGui::Spacing( );
	ImGui::Spacing( );

	std::vector < const char* > PitchModes = { _S( "None" ), _S( "Down" ), _S( "Up" ), _S( "Fake down" ), _S( "Fake up" ) };
	ImGui::SingleSelect( _S( "Pitch" ), g_Settings->m_iPitchMode, PitchModes );

	ImGui::SliderInt( _S( "Left fake limit" ), g_Settings->m_iLeftFakeLimit, 0, 60 );
	ImGui::SliderInt( _S( "Right fake limit" ), g_Settings->m_iRightFakeLimit, 0, 60 );
	
	std::vector < std::string > InverterConditions = { _S( "Stand" ), _S( "Move" ), _S( "Air" ) };
	if ( ImGui::BeginCombo (_S( "Auto inverter" ), _S( "Select" ), 0, InverterConditions.size( ) ) )
	{
		for ( int i = 0; i < InverterConditions.size( ); i++ )
			ImGui::Selectable( InverterConditions[ i ].c_str( ), &g_Settings->m_aInverterConditions[ i ], ImGuiSelectableFlags_DontClosePopups );

		ImGui::EndCombo( );
	}

	ImGui::Text( _S( "Inverter" ) );
	ImGui::Keybind( _S( "InvertButton" ), &g_Settings->m_aInverter->m_iKeySelected, &g_Settings->m_aInverter->m_iModeSelected );

	ImGui::EndChild( );
	ImGui::SameLine();
	//ImGui::SetNextWindowPos( ImVec2( iChildPosSecondX, iChildPosFirstY ) );
	ImGui::BeginChild( _S( "Fakelag settings" ), ImVec2( iChildDoubleSizeX, 180) );

	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox( _S( "Enable fakelag" ), g_Settings->m_bFakeLagEnabled );
	
	ImGui::SliderInt( _S( "Lag limit" ), g_Settings->m_iLagLimit, 1, 14 );

	std::vector < std::string > aLagTriggers
	=
	{
		_S( "Move" ),
		_S( "Air" ),
		_S( "Peek" )
	};

	if ( ImGui::BeginCombo (_S( "Lag triggers" ), _S( "Select" ), 0, aLagTriggers.size( ) ) )
	{
		for ( int i = 0; i < aLagTriggers.size( ); i++ )
			ImGui::Selectable( aLagTriggers[ i ].c_str( ), &g_Settings->m_aFakelagTriggers[ i ], ImGuiSelectableFlags_DontClosePopups );

		ImGui::EndCombo( );
	}

	ImGui::SliderInt( _S( "Trigger limit" ), g_Settings->m_iTriggerLimit, 1, 14 );

	ImGui::EndChild( );

	ImGui::SetCursorPos({ 534,288 + tab_padding });
	ImGui::BeginChild( _S( "Movement" ), ImVec2( iChildDoubleSizeX, 150 ) );
		
	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox( _S( "Jitter move" ), g_Settings->m_bJitterMove );

	ImGui::Text( _S( "Slowwalk" ) );
	ImGui::Keybind( _S( "SW" ), &g_Settings->m_aSlowwalk->m_iKeySelected, &g_Settings->m_aSlowwalk->m_iModeSelected );

	ImGui::Text( _S( "Fakeduck" ) );
	ImGui::Keybind( _S( "FD" ), &g_Settings->m_aFakeDuck->m_iKeySelected, &g_Settings->m_aFakeDuck->m_iModeSelected );

	ImGui::Text( _S( "Auto peek" ) );
	ImGui::Keybind( _S( "AP" ), &g_Settings->m_aAutoPeek->m_iKeySelected, &g_Settings->m_aAutoPeek->m_iModeSelected );

	std::vector < const char* > aLegMovement
	=
	{
		_S( "Default" ),
		_S( "Slide" )
	};

	ImGui::SingleSelect( _S( "Leg movement" ), g_Settings->m_iLegMovement, aLegMovement );

	ImGui::EndChild( );
}

void C_Menu::DrawInventoryTab()
{
	static int current_profile = -1;
	
// hey stewen, what r u doing there? he, hm heee, DRUGS
	static bool drugs = false;

	// some animation logic(switch)
	static bool active_animation = false;
	static bool preview_reverse = false;
	static float switch_alpha = 1.f;
	static int next_id = -1;
	if (active_animation)
	{
		if (preview_reverse)
		{
			if (switch_alpha == 1.f) //-V550
			{
				preview_reverse = false;
				active_animation = false;
			}

			switch_alpha = ImClamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
		}
		else
		{
			if (switch_alpha == 0.01f) //-V550
			{
				preview_reverse = true;
			}

			switch_alpha = ImClamp(switch_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
		}
	}
	else
		switch_alpha = ImClamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.0f, 1.f);

	// add
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;

	int iChildSizeX = (800 - ImGui::CalcTextSize(this->GetMenuName()).x - 40 - 30);
	int iChildSizeY = (565 - 30);

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 15 + 40;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 40;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;
	ImGui::PopFont();
	// add

	//ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));

	{
		ImGui::BeginChild(_S("Weapons Select"), ImVec2(768, 363));
		{
			//child_title(current_profile == -1 ? _S("Skinchanger") : game_data::weapon_names[current_profile].name);
			// we need to count our items in 1 line
			auto same_line_counter = 0;

			// if we didnt choose any weapon
			if (current_profile == -1)
			{
				for (auto i = 0; i < g_Settings->skins.skinChanger.size(); i++)
				{
					// do we need update our preview for some reasons?
					if (!all_skins[i])
					{
						g_Settings->skins.skinChanger.at(i).update();
						all_skins[i] = get_skin_preview(get_wep(i, (i == 0 || i == 1) ? g_Settings->skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), g_Settings->skins.skinChanger.at(i).skin_name, g_Globals.m_Interfaces.m_DirectDevice); //-V810
					}

					// we licked on weapon
					if (ImGui::ImageButton(all_skins[i], ImVec2(110, 76)))
					{
						next_id = i;
						active_animation = true;
					}

					// if our animation step is half from all - switch profile
					if (active_animation && preview_reverse)
					{
						ImGui::SetScrollY(0);
						current_profile = next_id;
					}

					if (same_line_counter < 5) { // continue push same-line
						ImGui::SameLine();
						same_line_counter++;
					}
					else { // we have maximum elements in 1 line
						same_line_counter = 0;
					}
				}
			}
			else
			{

				// update skin preview bool
				static bool need_update[36];

				// we pressed _S("Save & Close") button
				static bool leave;

				// update if we have nullptr texture or if we push force update
				if (!all_skins[current_profile] || need_update[current_profile])
				{
					all_skins[current_profile] = get_skin_preview(get_wep(current_profile, (current_profile == 0 || current_profile == 1) ? g_Settings->skins.skinChanger.at(current_profile).definition_override_vector_index : -1, current_profile == 0).c_str(), g_Settings->skins.skinChanger.at(current_profile).skin_name, g_Globals.m_Interfaces.m_DirectDevice); //-V810
					need_update[current_profile] = false;
				}

				// get settings for selected weapon
				auto& selected_entry = g_Settings->skins.skinChanger[current_profile];
				selected_entry.itemIdIndex = current_profile;

				ImGui::Columns(2,nullptr,false);

				ImGui::BeginChild(_S("Weapon Skins"), ImVec2(280, 455));
				{

					ImGui::BeginGroup();
					ImGui::PushItemWidth(230);

					// search input later
					static char search_skins[64] = "\0";
					static auto item_index = selected_entry.paint_kit_vector_index;

					if (!current_profile)
					{
						//ImGui::SetCursorPosX(-40);//padding
						if (ImGui::Combo(_S("Knife"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
							{
								*out_text = game_data::knife_names[idx].name;
								return true;
							}, nullptr, IM_ARRAYSIZE(game_data::knife_names)))
							need_update[current_profile] = true; // push force update
					}
					else if (current_profile == 1)
					{
						//ImGui::SetCursorPosX(-40);//padding
						if (ImGui::Combo(_S("Gloves"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
							{
								*out_text = game_data::glove_names[idx].name;
								return true;
							}, nullptr, IM_ARRAYSIZE(game_data::glove_names)))
						{
							item_index = 0; // set new generated paintkits element to 0;
							need_update[current_profile] = true; // push force update
						}
					}
					else
						selected_entry.definition_override_vector_index = 0;

					if (current_profile != 1)
					{
						ImGui::Text(_S("Search"));
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

						if (ImGui::InputText(_S("##search"), search_skins, sizeof(search_skins)))
							item_index = -1;

					}

					auto main_kits = current_profile == 1 ? SkinChanger::gloveKits : SkinChanger::skinKits;
					auto display_index = 0;

					SkinChanger::displayKits = main_kits;

					// we dont need custom gloves
					if (current_profile == 1)
					{
						for (auto i = 0; i < main_kits.size(); i++)
						{
							auto main_name = main_kits.at(i).name;

							for (auto i = 0; i < main_name.size(); i++)
								if (iswalpha((main_name.at(i))))
									main_name.at(i) = towlower(main_name.at(i));

							char search_name[64];

							if (!strcmp(game_data::glove_names[selected_entry.definition_override_vector_index].name, _S("Hydra")))
								strcpy_s(search_name, sizeof(search_name), _S("Bloodhound"));
							else
								strcpy_s(search_name, sizeof(search_name), game_data::glove_names[selected_entry.definition_override_vector_index].name);

							for (auto i = 0; i < sizeof(search_name); i++)
								if (iswalpha(search_name[i]))
									search_name[i] = towlower(search_name[i]);

							if (main_name.find(search_name) != std::string::npos)
							{
								SkinChanger::displayKits.at(display_index) = main_kits.at(i);
								display_index++;
							}
						}

						SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
					}
					else
					{
						if (strcmp(search_skins, _S(""))) //-V526
						{
							for (auto i = 0; i < main_kits.size(); i++)
							{
								auto main_name = main_kits.at(i).name;

								for (auto i = 0; i < main_name.size(); i++)
									if (iswalpha(main_name.at(i)))
										main_name.at(i) = towlower(main_name.at(i));

								char search_name[64];
								strcpy_s(search_name, sizeof(search_name), search_skins);

								for (auto i = 0; i < sizeof(search_name); i++)
									if (iswalpha(search_name[i]))
										search_name[i] = towlower(search_name[i]);

								if (main_name.find(search_name) != std::string::npos)
								{
									SkinChanger::displayKits.at(display_index) = main_kits.at(i);
									display_index++;
								}
							}

							SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
						}
						else
							item_index = selected_entry.paint_kit_vector_index;
					}

					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
					if (!SkinChanger::displayKits.empty())
					{
						if (ImGui::ListBox(_S("##PAINTKITS"), &item_index, [](void* data, int idx, const char** out_text) //-V107
							{
								while (SkinChanger::displayKits.at(idx).name.find(_S("ё")) != std::string::npos) //-V807
									SkinChanger::displayKits.at(idx).name.replace(SkinChanger::displayKits.at(idx).name.find(_S("ё")), 2, _S("е"));

								*out_text = SkinChanger::displayKits.at(idx).name.c_str();
								return true;
							}, nullptr, SkinChanger::displayKits.size(), SkinChanger::displayKits.size() > 9 ? 9 : SkinChanger::displayKits.size()) || !all_skins[current_profile])
						{
							SkinChanger::scheduleHudUpdate();
							need_update[current_profile] = true;

							auto i = 0;

							while (i < main_kits.size())
							{
								if (main_kits.at(i).id == SkinChanger::displayKits.at(item_index).id)
								{
									selected_entry.paint_kit_vector_index = i;
									break;
								}

								i++;
							}

						}
					}
					ImGui::PopStyleVar();


					ImGui::PushItemWidth(190);
					if (ImGui::InputInt(_S("Seed"), &selected_entry.seed, 1, 100))
						SkinChanger::scheduleHudUpdate();

					if (ImGui::InputInt(_S("StatTrak"), &selected_entry.stat_trak, 1, 15))
						SkinChanger::scheduleHudUpdate();
					ImGui::PopItemWidth();

					//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6);
					//ImGui::Text(_S("Quality"));
					//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);


					//ImGui::SetCursorPosX(-40);//padding
					if (ImGui::Combo(_S("Quality"), &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = game_data::quality_names[idx].name;
						return true;
					}, nullptr, IM_ARRAYSIZE(game_data::quality_names)))
					SkinChanger::scheduleHudUpdate();

					//ImGui::SetCursorPosX(-20);//padding
					if (ImGui::SliderFloat(_S("Wear"), &selected_entry.wear, 0.0f, 1.0f))
						drugs = true;
					else if (drugs)
					{
						SkinChanger::scheduleHudUpdate();
						drugs = false;
					}

						if (current_profile != 1)
						{
							if (!g_Settings->skins.custom_name_tag[current_profile].empty())
								strcpy_s(selected_entry.custom_name, sizeof(selected_entry.custom_name), g_Settings->skins.custom_name_tag[current_profile].c_str());

							ImGui::Text(_S("Name Tag"));
							ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

							if (ImGui::InputText(_S("##nametag"), selected_entry.custom_name, sizeof(selected_entry.custom_name)))
							{
								g_Settings->skins.custom_name_tag[current_profile] = selected_entry.custom_name;
								SkinChanger::scheduleHudUpdate();
							}

							ImGui::PopStyleVar();
						}

						ImGui::PopItemWidth();

						ImGui::EndGroup();

				}
				ImGui::EndChild();

				ImGui::NextColumn();

				ImGui::BeginChild(_S("##SKINCHANGER__2CHILD"), ImVec2(220, 450));
				{

					ImGui::BeginGroup();
					if (ImGui::ImageButton(all_skins[current_profile], ImVec2(190, 155)))
					{
						// maybe i will do smth later where, who knows :/
					}

					if (ImGui::CustomButton(_S("Close"), ImVec2(198, 26)))
					{
						// start animation
						active_animation = true;
						next_id = -1;
						leave = true;
					}
					ImGui::EndGroup();

				}ImGui::EndChild();
					// update element
					//selected_entry.update();

					// we need to reset profile in the end to prevent render images with massive's index == -1
					if (leave && (preview_reverse || !active_animation))
					{
						ImGui::SetScrollY(0);
						current_profile = next_id;
						leave = false;
					}

			}
		}
		ImGui::EndChild();
	}
}

void C_Menu::DrawPlayersTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;
	ImGui::PopFont();

	//ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild("ESP", ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));
	static int iPlayerESPType = 0;
		
	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::SingleSelect(_S("Player Type"), &iPlayerESPType, { _S("Enemy"), _S("Team"), _S("Local") });

	static C_PlayerSettings* Settings = NULL;
	switch ( iPlayerESPType )
	{
		case 0: Settings = g_Settings->m_Enemies; break;
		case 1: Settings = g_Settings->m_Teammates; break;
		case 2: Settings = g_Settings->m_LocalPlayer; break;
	}

	ImGui::Checkbox(_S("Box"), &Settings->m_BoundaryBox);
	this->DrawColorEdit4(_S("Box##color"), &Settings->m_aBoundaryBox);

	ImGui::Checkbox(_S("Name"), &Settings->m_RenderName);
	this->DrawColorEdit4(_S("Name##color"), &Settings->m_aNameColor);

	ImGui::Checkbox(_S("Health bar"), &Settings->m_RenderHealthBar);
	this->DrawColorEdit4(_S("m_aHealth##color"), &Settings->m_aHealthBar);

	ImGui::Checkbox(_S("Health bar text"), &Settings->m_RenderHealthText);
	this->DrawColorEdit4(_S("m_aHealthText##color"), &Settings->m_aHealthText);

	ImGui::Checkbox(_S("Ammo bar"), &Settings->m_RenderAmmoBar);
	this->DrawColorEdit4(_S("m_aAmmoBar##color"), &Settings->m_aAmmoBar);

	ImGui::Checkbox(_S("Ammo bar text"), &Settings->m_RenderAmmoBarText);
	this->DrawColorEdit4(_S("m_aAmmoBarText##color"), &Settings->m_aAmmoBarText);

	ImGui::Checkbox(_S("Weapon Text"), &Settings->m_RenderWeaponText);
	this->DrawColorEdit4(_S("m_aWeaponText##color"), &Settings->m_aWeaponText);

	ImGui::Checkbox(_S("Weapon Icon"), &Settings->m_RenderWeaponIcon);
	this->DrawColorEdit4(_S("m_aWeaponIcon##color"), &Settings->m_aWeaponIcon);
	
	ImGui::Checkbox(_S("Out of view arrows"), g_Settings->m_bOutOfViewArrows);
	this->DrawColorEdit4(_S("awerqweqw2e123412er412q4##color"), g_Settings->m_aOutOfViewArrows);

	const char* aFlags[ 5 ] 
	=
	{
		"Scoped",
		"Armor",
		"Flashed",
		"Location",
		"Money"
	};

	for ( int i = 0; i < 5; i++ )
	{
		ImGui::Checkbox( aFlags [ i ], &Settings->m_Flags[ i ] );
		this->DrawColorEdit4( ( "##" + std::to_string( i ) ).c_str( ), &Settings->m_Colors[ i ] );
	}

	ImGui::EndChild();

	ImGui::SameLine();

	//ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY + iChildSizeY - 240 + 15));
	ImGui::BeginChild( _S( "Glow" ), ImVec2( iChildDoubleSizeX, 225 ) );
	
	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox( _S( "Player Glow" ), &Settings->m_bRenderGlow );
	this->DrawColorEdit4( _S( "##m_aGlowcolor" ), &Settings->m_aGlow );
	ImGui::SingleSelect( _S( "Glow style##1" ), &Settings->m_iGlowStyle, { _S( "Outline" ), _S( "Thin" ), _S( "Cover" ), _S( "Cover Pulse" ) } );

	ImGui::Checkbox( _S("C4 Glow" ), g_Settings->m_bRenderC4Glow );
	this->DrawColorEdit4( _S( "##C4Glowcolor" ), g_Settings->m_aC4Glow );
	ImGui::SingleSelect( _S( "Glow style##m_iC4GlowStyle" ), g_Settings->m_iC4GlowStyle, { _S( "Outline" ), _S( "Thin" ), _S( "Cover" ), _S( "Cover Pulse" ) } );

	ImGui::Checkbox( _S("Dropped Weapon Glow" ), g_Settings->m_bRenderDroppedWeaponGlow);
	this->DrawColorEdit4( _S("m_aDroppedWeaponGlow##color" ), g_Settings->m_aDroppedWeaponGlow);
	ImGui::SingleSelect( _S( "Glow style##m_iDroppedWeaponGlowStyle" ), g_Settings->m_iDroppedWeaponGlowStyle, { _S( "Outline" ), _S( "Thin" ), _S( "Cover" ), _S( "Cover Pulse" ) } );

	ImGui::Checkbox( _S( "Projectiles Glow" ), g_Settings->m_bRenderProjectileGlow );
	this->DrawColorEdit4( _S( "##Projectile" ), g_Settings->m_aProjectileGlow );
	ImGui::SingleSelect( _S( "Glow style##m_iProjectileGlowStyle" ), g_Settings->m_iProjectileGlowStyle, { _S( "Outline" ), _S( "Thin" ), _S( "Cover" ), _S( "Cover Pulse" ) } );

	ImGui::EndChild();

	g_DrawModel->SetGlow( Settings->m_iGlowStyle );
	if ( !Settings->m_bRenderGlow )
		g_DrawModel->SetGlow( -1 );
	else
		g_DrawModel->SetGlowColor( Color( Settings->m_aGlow ) );

	//ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosFirstY));
	ImGui::SameLine();
	ImGui::BeginChild(_S("Chams"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::Spacing( );
	ImGui::Spacing( );

	static int32_t iChamsGroup = 0;
	ImGui::SingleSelect( _S( "Chams group" ), &iChamsGroup, { _S( "Enemy visible" ), _S( "Enemy invisble" ), _S( "Backtrack" ), _S( "Shot chams" ), _S( "Team visible" ), _S( "Team invisible" ), _S( "Local" ), _S( "Desync" ), _S( "Lag" ), _S("Hands")});

	ImGui::Checkbox( _S( "Enable Chams" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_bRenderChams );
	this->DrawColorEdit4( _S( "##qweqwe" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_Color );
	ImGui::SingleSelect( _S( "Material" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_iMainMaterial, { _S( "Flat" ), _S( "Regular" ) } );

	ImGui::Checkbox( _S( "Enable glow" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_aModifiers[ 0 ] );
	this->DrawColorEdit4( _S( "##512414 color" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_aModifiersColors[ 0 ] );
	ImGui::Checkbox( _S( "Enable ghost" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_aModifiers[ 1 ] );
	this->DrawColorEdit4( _S( "##235235 color" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_aModifiersColors[ 1 ] );
	ImGui::Checkbox( _S( "Enable glass" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_aModifiers[ 2 ] );
	this->DrawColorEdit4( _S( "##4124124 color" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_aModifiersColors[ 2 ] );
	ImGui::Checkbox( _S( "Enable pulsation" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_aModifiers[ 3 ] );
	this->DrawColorEdit4( _S( "##123123 color" ), &g_Settings->m_aChamsSettings[ iChamsGroup ].m_aModifiersColors[ 3 ] );
	ImGui::Checkbox( _S( "Ragdoll chams" ), g_Settings->m_bDrawRagdolls );

	if ( iChamsGroup < 8 )
		g_DrawModel->SetChamsSettings( g_Settings->m_aChamsSettings[ iChamsGroup ] );

	ImGui::EndChild();

	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 654 + 150, vecWindowPosition.y), ImVec2(vecWindowPosition.x + 950 + 150, vecWindowPosition.y + 440), ImColor(26, 26, 30, 255), 4.0f);
	if (g_DrawModel->GetTexture())
	{
		ImGui::GetForegroundDrawList()->AddImage(
			g_DrawModel->GetTexture()->pTextureHandles[0]->lpRawTexture,
			ImVec2(vecWindowPosition.x + 610 + 150, vecWindowPosition.y - 130),
			ImVec2(vecWindowPosition.x + 610 + 150 + g_DrawModel->GetTexture()->GetActualWidth(), vecWindowPosition.y + g_DrawModel->GetTexture()->GetActualHeight() - 130),
			ImVec2(0, 0), ImVec2(1, 1),
			ImColor(1.0f, 1.0f, 1.0f, 1.0f));
	}

	static ImVec2 vecPreviousMousePosition = ImVec2(0, 0);
	static ImVec2 vecLastMousePosition = ImVec2(0, 0);
	ImVec2 vecCurrentCursorPosition = ImGui::GetMousePos();

	static bool bIsActive = false;

	// render box
	Color aBox = Color(Settings->m_aBoundaryBox);
	if (Settings->m_BoundaryBox)
	{
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 694 + 150, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 886 + 170, vecWindowPosition.y + 386), ImColor(0, 0, 0, 255));
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 696 + 150, vecWindowPosition.y + 41), ImVec2(vecWindowPosition.x + 884 + 170, vecWindowPosition.y + 384), ImColor(0, 0, 0, 255));
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 885 + 170, vecWindowPosition.y + 385), ImColor(aBox.r(), aBox.g(), aBox.b(), aBox.a()));
	}

	// render name
	Color aName = Color(Settings->m_aNameColor);
	if (Settings->m_RenderName)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("Agent Ava")).x / 2, vecWindowPosition.y + 22), ImColor(aName.r(), aName.g(), aName.b(), aName.a()), _S("Agent Ava"));
		ImGui::PopFont();
	}

	// render health
	Color aHealthBar = Color(Settings->m_aHealthBar);
	if (Settings->m_RenderHealthBar)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 689 + 150, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 693 + 150, vecWindowPosition.y + 385), ImColor(0, 0, 0, 100));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 690 + 150, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 692 + 150, vecWindowPosition.y + 385), ImColor(aHealthBar.r(), aHealthBar.g(), aHealthBar.b(), aHealthBar.a()));
	}

	Color aHealthText = Color(Settings->m_aHealthText);
	if (Settings->m_RenderHealthText)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);

		if (Settings->m_RenderHealthBar)
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 150 - ImGui::CalcTextSize(_S("100")).x, vecWindowPosition.y + 37 ), ImColor(aHealthText.r(), aHealthText.g(), aHealthText.b(), aHealthText.a()), _S("100"));
		else
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 691 + 150 - ImGui::CalcTextSize(_S("100")).x, vecWindowPosition.y + 37 ), ImColor(aHealthText.r(), aHealthText.g(), aHealthText.b(), aHealthText.a()), _S("100"));

		ImGui::PopFont();
	}

	Color aWeaponText = Color(Settings->m_aWeaponText);
	if (Settings->m_RenderWeaponText)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("P2000")).x / 2, vecWindowPosition.y + 385 + 6), ImColor(aWeaponText.r(), aWeaponText.g(), aWeaponText.b(), aWeaponText.a()), _S("P2000"));
		ImGui::PopFont();
	}

	Color aWeaponIcon = Color(Settings->m_aWeaponIcon);
	if (Settings->m_RenderWeaponIcon)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_WeaponIcon);

		if (Settings->m_RenderWeaponText)
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("E")).x / 2, vecWindowPosition.y + 385 + 22), ImColor(aWeaponIcon.r(), aWeaponIcon.g(), aWeaponIcon.b(), aWeaponIcon.a()), _S("E"));
		else
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("E")).x / 2, vecWindowPosition.y + 385 + 8), ImColor(aWeaponIcon.r(), aWeaponIcon.g(), aWeaponIcon.b(), aWeaponIcon.a()), _S("E"));

		ImGui::PopFont();
	}

	Color aAmmoBar = Color(Settings->m_aAmmoBar);
	if (Settings->m_RenderAmmoBar)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 387), ImVec2(vecWindowPosition.x + 906 + 150, vecWindowPosition.y + 391), ImColor(0, 0, 0, 100));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 388), ImVec2(vecWindowPosition.x + 905 + 150, vecWindowPosition.y + 390), ImColor(aAmmoBar.r(), aAmmoBar.g(), aAmmoBar.b(), aAmmoBar.a()));
	}

	Color aAmmoText = Color(Settings->m_aAmmoBarText);
	if (Settings->m_RenderAmmoBarText)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 902 + 150 + ImGui::CalcTextSize(_S("13")).x / 2, vecWindowPosition.y + 386), ImColor(aAmmoText.r(), aAmmoText.g(), aAmmoText.b(), aAmmoText.a()), _S("13"));
		ImGui::PopFont();
	}
}

static int Selected = 0;

void C_Menu::DrawMiscTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;

	ImGui::PopFont();

	//ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild(_S("Movement"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox(_S("Auto jump"), g_Settings->m_bBunnyHop);
	ImGui::Checkbox(_S("Auto strafe"), g_Settings->m_bAutoStrafe);
	ImGui::Checkbox(_S("WASD strafe"), g_Settings->m_bWASDStrafe);
	ImGui::Checkbox(_S("Start speed"), g_Settings->m_bSpeedBoost);
	ImGui::Checkbox(_S("Quick stop"), g_Settings->m_bFastStop);
	ImGui::Checkbox(_S("Edge jump"), g_Settings->m_bEdgeJump);
	ImGui::Checkbox(_S("Infinity duck"), g_Settings->m_bInfinityDuck);
	ImGui::EndChild();

	ImGui::SameLine();
	//ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosFirstY));
	ImGui::BeginChild(_S("Other"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox(_S("Anti Untrusted"), g_Settings->m_bAntiUntrusted);

	std::vector < std::string > aLogItems = { _S( "Hurt" ), _S( "Harm" ), _S( "Purchase" ), _S( "Bomb" ), _S( "Miss") };
	if (ImGui::BeginCombo(_S("Event logs"), _S("Select"), ImGuiComboFlags_HeightSmall, aLogItems.size()))
	{
		ImGui::Selectable(aLogItems[0].c_str(), g_Settings->m_bLogHurts.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
		ImGui::Selectable(aLogItems[1].c_str(), g_Settings->m_bLogHarms.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
		ImGui::Selectable(aLogItems[2].c_str(), g_Settings->m_bLogPurchases.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
		ImGui::Selectable(aLogItems[3].c_str(), g_Settings->m_bLogBomb.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
		ImGui::Selectable(aLogItems[4].c_str(), g_Settings->m_bLogMisses.GetPtr(), ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	ImGui::Checkbox( _S( "Filter server ads" ), g_Settings->m_bAdBlock );
	ImGui::Checkbox( _S( "Filter console" ), g_Settings->m_bFilterConsole );
	ImGui::Checkbox( _S( "Unlock convars" ), g_Settings->m_bUnhideConvars );
	ImGui::Checkbox( _S( "Reveal ranks" ), g_Settings->m_bRevealRanks );
	ImGui::Checkbox( _S( "Unlock inventory" ), g_Settings->m_bUnlockInventoryAccess );
	
	ImGui::Checkbox( _S( "Spectator list" ), g_Settings->m_bSpectatorList );
	ImGui::Checkbox( _S( "Keybind list" ), g_Settings->m_bDrawKeyBindList );
	ImGui::Checkbox( _S( "Watermark" ), g_Settings->m_bWaterMark );
	ImGui::Checkbox( _S( "Clantag changer" ), g_Settings->m_bTagChanger );

	ImGui::EndChild();
	ImGui::SameLine();
	//ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosSecondY));
	ImGui::BeginChild(_S("BuyBot"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));
	
	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox(_S("Enable BuyBot"), g_Settings->m_bBuyBotEnabled);

	std::vector < std::string > aEquipment
		=
	{
	_S("Fire grenade/Molotov"),
	_S("Smoke grenade"),
	_S("Flash grenade"),
	_S("Explosive grenade"),
	_S("Taser"),
	_S("Heavy armor"),
	_S("Helmet"),
	_S("Defuser")
	};

	ImGui::SingleSelect
	(
		_S("Primary"),
		g_Settings->m_BuyBotPrimaryWeapon,
		{
			_S("None"),
			_S("SCAR20/G3SG1"),
			_S("Scout"),
			_S("AWP"),
			_S("M4A1/AK47")
		}
	);
	ImGui::SingleSelect(
		_S("Secondary"),
		g_Settings->m_BuyBotSecondaryWeapon,
		{
			_S("None"),
			_S("FN57/TEC9"),
			_S("Dual elites"),
			_S("Deagle/Revolver"),
			_S("P2000/Glock-18"),
			_S("P250")
		}
	);

	if (ImGui::BeginCombo(_S("Equipment"), _S("Select"), 0, 8))
	{
		for (int i = 0; i < aEquipment.size(); i++)
			ImGui::Selectable(aEquipment[i].c_str(), &g_Settings->m_aEquipment[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	ImGui::Checkbox(_S("Don't override AWP"), g_Settings->m_bBuyBotKeepAWP);

	ImGui::EndChild();
}

void C_Menu::DrawWorldTab( )
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;
	ImGui::PopFont();

	ImGui::BeginChild(_S("View settings"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));
	ImGui::Spacing( );
	ImGui::Spacing( );
	ImGui::Text( _S( "Thirdperson" ) );
	ImGui::Keybind( _S( "ThirdPerson Bind" ), &g_Settings->m_aThirdPerson->m_iKeySelected, &g_Settings->m_aThirdPerson->m_iModeSelected );
	ImGui::SliderInt( _S( "Distance" ), g_Settings->m_iThirdPersonDistance, 50, 300 );
	ImGui::SliderInt( _S( "Camera distance" ), g_Settings->m_iCameraDistance, 90, 140 );
	ImGui::Checkbox( _S( "Force distance while scoped" ), g_Settings->m_bOverrideFOVWhileScoped );
	ImGui::SliderFloat( _S( "Aspect ratio" ), g_Settings->m_flAspectRatio, 0.01f, 3.0f );

	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild (_S( "Viewmodel" ), ImVec2( iChildDoubleSizeX, iChildDoubleSizeY ) );
	ImGui::Spacing( );
	ImGui::Spacing( );
	ImGui::SliderInt( _S( "Viewmodel distance" ), g_Settings->m_iViewmodelDistance, 60, 140 );
	ImGui::SliderInt( _S( "Viewmodel X axis" ), g_Settings->m_iViewmodelX, -10, 10 );
	ImGui::SliderInt( _S( "Viewmodel Y axis" ), g_Settings->m_iViewmodelY, -10, 10 );
	ImGui::SliderInt( _S( "Viewmodel Z axis" ), g_Settings->m_iViewmodelZ, -10, 10 );
	ImGui::SliderInt( _S( "Viewmodel roll" ), g_Settings->m_iViewmodelRoll, -90, 90 );
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild (_S( "World modulation" ), ImVec2( iChildDoubleSizeX, iChildDoubleSizeY ) );
	ImGui::Spacing( );
	ImGui::Spacing( );

	std::vector < const char* > aSkyboxList =
	{
		_S( "None" ),
		_S( "Tibet" ),
		_S( "Baggage" ),
		_S( "Italy" ),
		_S( "Aztec" ),
		_S( "Vertigo" ),
		_S( "Daylight" ),
		_S( "Daylight 2" ),
		_S( "Clouds" ),
		_S( "Clouds 2" ),
		_S( "Gray" ),
		_S( "Clear" ),
		_S( "Canals" ),
		_S( "Cobblestone" ),
		_S( "Assault" ),
		_S( "Clouds dark" ),
		_S( "Night" ),
		_S( "Night 2" ),
		_S( "Night flat" ),
		_S( "Dusty" ),
		_S( "Rainy" ),
		_S( "Custom" )
	};

	ImGui::Checkbox( _S( "Hold fire animation" ), g_Settings->m_bHoldFireAnimation );
	ImGui::Checkbox( _S( "Preserve killfeed" ), g_Settings->m_bPreserveKillfeed );
	ImGui::SingleSelect( _S( "Skybox Changer" ), g_Settings->m_iSkybox.GetPtr( ), aSkyboxList );
	if ( g_Settings->m_iSkybox == aSkyboxList.size( ) - 1 )
	{
		static char aSkyBox[ 32 ];
		if ( !g_Settings->m_szCustomSkybox->empty( ) )
			strcpy( aSkyBox, g_Settings->m_szCustomSkybox.Get( ).c_str( ) );

		if ( ImGui::InputText( _S( "##324234124" ), aSkyBox, 32 ) )
			g_Settings->m_szCustomSkybox.Get( ) = aSkyBox;
	}

	ImGui::Text( _S( "World color" ) );
	this->DrawColorEdit4( _S( "##123123" ), g_Settings->m_WorldModulation );

	ImGui::Text( _S( "Props color" ) );
	this->DrawColorEdit4( _S( "##11233" ), g_Settings->m_PropModulation );

	ImGui::Text( _S( "Skybox color" ) );
	this->DrawColorEdit4( _S( "##51223" ), g_Settings->m_SkyModulation );
	
	ImGui::Spacing( );

	std::vector < const char* > aHitSounds =
	{
		"Metallic",
		"Bell"
	};

	ImGui::Checkbox( _S( "Penetration crosshair" ), g_Settings->m_bPenetrationCrosshair );
	ImGui::Checkbox( _S( "Force crosshair" ), g_Settings->m_bForceCrosshair );
	ImGui::Checkbox( _S( "Hitmarker" ), g_Settings->m_bHitMarker );
	ImGui::Checkbox( _S( "Hitsound" ), g_Settings->m_bHitSound );
	ImGui::SingleSelect( "##1412412", g_Settings->m_nHitSound, aHitSounds );
	ImGui::Checkbox( _S( "Damage marker" ), g_Settings->m_bDamageMarker);

	ImGui::Checkbox( _S( "Client bullet impacts" ), g_Settings->m_bDrawClientImpacts );
	this->DrawColorEdit4( _S( "##41242354" ), g_Settings->m_ClientImpacts );
	ImGui::Checkbox( _S( "Server bullet impacts" ), g_Settings->m_bDrawServerImpacts );
	this->DrawColorEdit4( _S( "##412423154" ), g_Settings->m_ServerImpacts );

	ImGui::Checkbox( _S( "Local bullet tracers" ), g_Settings->m_bDrawLocalTracers );
	this->DrawColorEdit4( _S( "##43242354" ), g_Settings->m_LocalTracers );
	ImGui::Checkbox( _S( "Enemy bullet tracers" ), g_Settings->m_bDrawEnemyTracers );
	this->DrawColorEdit4( _S( "##432423154" ), g_Settings->m_EnemyTracers );

	std::vector < std::string > aWorldRemovals =
	{
		_S( "Visual punch" ),
		_S( "Visual kick" ),
		_S( "Scope" ),
		_S( "Smoke" ),
		_S( "Flash" ),
		_S( "Post process" ),
		_S( "World FOG" ),
		_S( "Shadows" ),
		_S( "Landing bob" ),
		_S( "Hand shaking" )
	};

	ImGui::Checkbox( _S( "Grenade prediction" ), g_Settings->m_bPredictGrenades );
	this->DrawColorEdit4( _S( "##1234142124" ), g_Settings->m_GrenadeWarning );

	ImGui::Checkbox( _S( "Grenade timers" ), g_Settings->m_GrenadeTimers );
	this->DrawColorEdit4( _S( "##1234145151" ), g_Settings->m_GrenadeWarningTimer );

	if (ImGui::BeginCombo(_S("World removals"), _S("Select"), 0, 8))
	{
		for (int i = 0; i < aWorldRemovals.size(); i++)
			ImGui::Selectable(aWorldRemovals[i].c_str(), &g_Settings->m_aWorldRemovals[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	ImGui::EndChild( );
}

void C_Menu::DrawConfigTab( )
{	
	static std::string selected_cfg = "";
    static char cfg_name[32];

    ImGui::BeginChild(_S("Config Settings"), ImVec2(251, 363));
	{
        if (ImGui::InputText(_S(""), cfg_name, 32)) selected_cfg = std::string(cfg_name);
   
		if (ImGui::Button(_S("Create config"), ImVec2(232, 25)))
		{
			std::ofstream(selected_cfg + ".cfg", std::ios_base::trunc);
			g_ConfigSystem->SaveConfig((selected_cfg + (std::string)(".cfg")).c_str());
		}

		if (ImGui::Button(_S("Save config"), ImVec2(232, 25)))
			g_ConfigSystem->SaveConfig(selected_cfg.c_str());

		if (ImGui::Button(_S("Load config"), ImVec2(232, 25)))
			g_ConfigSystem->LoadConfig(selected_cfg.c_str());
	
	}
	ImGui::EndChild( );
	ImGui::SameLine();
	ImGui::BeginChild(_S("Config list"), ImVec2(251, 363));
	{
		{
			for (auto cfg : g_ConfigSystem->GetConfigList())
				if (ImGui::Selectable(cfg.c_str(), cfg == selected_cfg))
					selected_cfg = cfg;
		}
	}
	ImGui::EndChild();
}

void C_Menu::DrawColorEdit4( const char* strLabel, Color* aColor )
{
	 float aColour[ 4 ] =
		{
            aColor->r( ) / 255.0f,
			aColor->g( ) / 255.0f,
			aColor->b( ) / 255.0f,
			aColor->a( ) / 255.0f
        };

	 if ( ImGui::ColorEdit4( strLabel, aColour, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_RGB ) )
		 aColor->SetColor( aColour[ 0 ], aColour[ 1 ], aColour[ 2 ], aColour[ 3 ] );
}

void C_Menu::Initialize()
{
	ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = ImVec4(45 / 255.f, 45 / 255.f, 45 / 255.f, 1.f);
	ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab] = ImVec4(65 / 255.f, 65 / 255.f, 65 / 255.f, 1.f);
	ImGui::GetStyle().AntiAliasedFill = true;
	ImGui::GetStyle().AntiAliasedLines = true;
	ImGui::GetStyle().ScrollbarSize = 6;
	//ImGui::GetStyle().WindowRounding = 12;

	D3DXCreateTextureFromFileInMemoryEx(g_Globals.m_Interfaces.m_DirectDevice, &avatarka, sizeof(avatarka), 512, 512, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &g_Menu->m_dTexture);

	for (auto i = 0; i < g_Settings->skins.skinChanger.size(); i++)
		if (!all_skins[i])
			all_skins[i] = get_skin_preview(get_wep(i, (i == 0 || i == 1) ? g_Settings->skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), g_Settings->skins.skinChanger.at(i).skin_name, g_Globals.m_Interfaces.m_DirectDevice); //-V810

}

#include "../Features/Networking/Networking.hpp"
void C_Menu::WaterMark( )
{
	std::string szWatermark = _S( "zeeron.su | " ) + g_Globals.m_szUzername + _S( " |" );
	if ( !g_Settings->m_bWaterMark )
		return;

	C_NetChannelInfo* m_NetChannelInfo = g_Globals.m_Interfaces.m_EngineClient->GetNetChannelInfo( );
	if ( g_Globals.m_Interfaces.m_EngineClient->IsConnected( ) )
	{
		if ( m_NetChannelInfo )
		{
			if ( m_NetChannelInfo->IsLoopback( ) )
				szWatermark += _S( " local server |" );
			else
				szWatermark += " " + ( std::string )( m_NetChannelInfo->GetAddress( ) ) + " | ";
		
			szWatermark += ( std::string )( " " ) + std::to_string( ( int )( m_NetChannelInfo->GetAvgLatency( FLOW_OUTGOING ) + m_NetChannelInfo->GetAvgLatency( FLOW_INCOMING ) ) ) + ( std::string )( "ms |" );
		}
	}
	else
		szWatermark += _S( " unconnected |" );

	int nScreenSizeX, nScreenSizeY;
	g_Globals.m_Interfaces.m_EngineClient->GetScreenSize( nScreenSizeX, nScreenSizeY );
	
	int nTextLength = g_Globals.m_Fonts.m_SegoeUI->CalcTextSizeA( 16.0f, FLT_MAX, NULL, szWatermark.c_str( ) ).x + 75;
	szWatermark += _S( " FPS: " ) + std::to_string( ( int ) ( 1.0f / ImGui::GetIO( ).DeltaTime ) );
	
	ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( nScreenSizeX - nTextLength, 11 ), ImVec2( nScreenSizeX - 10, 30 ), ImColor( 24.0f / 255.0f, 31.0f / 255.0f, 44.0f / 255.0f, 1.0f ) );
	ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( nScreenSizeX - nTextLength, 30 ), ImVec2( nScreenSizeX - 10, 31 ), ImColor( 0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 255.0f / 255.0f ) );
	ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( nScreenSizeX - nTextLength, 11 ), ImVec2( nScreenSizeX - 10, 30 ), ImColor( 30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 100.0f / 255.0f ) );

	ImGui::PushFont( g_Globals.m_Fonts.m_SegoeUI );
	ImGui::GetOverlayDrawList( )->AddText( ImVec2( nScreenSizeX - nTextLength + 7, 12 ), ImColor( 255, 255, 255, 255 ), szWatermark.c_str( ) );
	ImGui::PopFont( );
}

#include "Tools/Tools.hpp"
#define PUSH_BIND( m_Variable, Name )\
if ( g_Tools->IsBindActive( m_Variable ) )\
{\
	if ( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent == 0.0f )\
		m_BindList[ FNV32( #m_Variable ) ].m_szName = _S( Name );\
	m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent = std::clamp( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent + ImGui::GetIO( ).DeltaTime * 10.0f, 0.0f, 1.0f );\
}\
else\
{\
	if ( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent == 0.0f )\
		m_BindList[ FNV32( #m_Variable ) ].m_szName = "";\
	m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent = std::clamp( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent - ImGui::GetIO( ).DeltaTime * 10.0f, 0.0f, 1.0f );\
}\

void C_Menu::DrawKeybindList( )
{
	if ( !g_Settings->m_bDrawKeyBindList )
		return;

	int m_Last = 0;
	PUSH_BIND( g_Settings->m_aFakeDuck, "Fake Duck" );
	PUSH_BIND( g_Settings->m_aDoubleTap, "Double tap" );
	PUSH_BIND( g_Settings->m_aSlowwalk, "Slow walk" );
	PUSH_BIND( g_Settings->m_aHideShots, "Hide shots" );
	PUSH_BIND( g_Settings->m_aSafePoint, "Safe points" );
	PUSH_BIND( g_Settings->m_aInverter, "Invert side" ); 
	PUSH_BIND( g_Settings->m_aAutoPeek, "Auto peek" );
	PUSH_BIND( g_Settings->m_aMinDamage, "Damage override" );

	int32_t iCount = 0;
	for ( auto& Bind : m_BindList )
	{
		if ( Bind.second.m_szName.length( ) )
			iCount++;
	}

	if ( iCount <= 0 && !m_bIsMenuOpened )
		return;

	int nAdvancedFlag = 0;
	if ( !m_bIsMenuOpened )
		nAdvancedFlag = ImGuiWindowFlags_NoMove;
	
	ImGui::SetNextWindowSize( ImVec2( 190, m_BindList.empty( ) ? 0 : 35 + ( 21.5f * iCount ) ) );
	ImGui::DefaultBegin( _S( "Keybind List" ), g_Settings->m_bDrawKeyBindList, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | nAdvancedFlag );
	{
		int32_t x = ImGui::GetCurrentWindow( )->Pos.x + 4.5f;
		int32_t y = ImGui::GetCurrentWindow( )->Pos.y;

		ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( x, y ), ImVec2( x + 181, y + 22 ), ImColor( 24.0f / 255.0f, 31.0f / 255.0f, 44.0f / 255.0f, 1.0f ) );
		ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( x, y + 22 ), ImVec2( x + 181, y + 22.27f ), ImColor( 0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 1.0f ) );
		ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( x, y + 22.27f ), ImVec2( x + 181, y + 22.30f ), ImColor( 0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 0.65f ) );

		ImGui::PushFont( g_Globals.m_Fonts.m_MenuIcons );
		ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 5, y + 2 ), ImColor( 71, 163, 255 ), _S( "a" ) );
		ImGui::PopFont( );

		ImGui::PushFont( g_Globals.m_Fonts.m_LogFont );
		ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 26, y + 2 ), ImColor( 255, 255, 255 ), _S( "Keybind list" ) );
		ImGui::PopFont( );

		for ( auto& Bind : m_BindList )
		{
			if ( !Bind.second.m_szName.length( ) )
				continue;
		
			ImGui::PushFont( g_Globals.m_Fonts.m_LogFont );
			ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 2, 27 + ( y + 16 * m_Last ) ), ImColor( 255, 255, 255, static_cast < int >( Bind.second.m_flAlphaPercent * 255.0f ) ), Bind.second.m_szName.c_str( ) );
			ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 160, 27 + ( y + 16 * m_Last ) ), ImColor( 255, 255, 255, static_cast < int >( Bind.second.m_flAlphaPercent * 255.0f ) ), _S( "On" ) );
			ImGui::PopFont( );

			m_Last++;
		}
		
	}
	ImGui::DefaultEnd( );
}

void C_Menu::DrawSpectatorList( )
{
	std::vector < std::string > vecSpectatorList;
	if ( !g_Settings->m_bSpectatorList )
		return;

	if ( g_Globals.m_LocalPlayer && g_Globals.m_LocalPlayer->IsAlive( ) )
	{
		for ( int nPlayerID = 1; nPlayerID <= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients; nPlayerID++ )
		{
			C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex( nPlayerID );
			if ( !pPlayer || pPlayer->IsAlive( ) || !pPlayer->IsPlayer( ) || pPlayer->IsDormant( ) || !pPlayer->m_hObserverTarget( ) )
				continue;

			C_PlayerInfo m_TargetInfo;
			g_Globals.m_Interfaces.m_EngineClient->GetPlayerInfo( pPlayer->EntIndex( ), &m_TargetInfo );

			vecSpectatorList.emplace_back( ( std::string )( m_TargetInfo.m_strName ) );
		}
	}

	if ( !m_bIsMenuOpened && vecSpectatorList.empty( ) )
		return;

	int nAdvancedFlag = 0;
	if ( !m_bIsMenuOpened )
		nAdvancedFlag = ImGuiWindowFlags_NoMove;
	
	ImGui::SetNextWindowSize( ImVec2( 190, m_BindList.empty( ) ? 0 : 35 + ( 21.5f * vecSpectatorList.size( ) ) ) );
	ImGui::DefaultBegin( _S( "Spectator List" ), g_Settings->m_bDrawKeyBindList, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | nAdvancedFlag );
	{
		int32_t x = ImGui::GetCurrentWindow( )->Pos.x + 4.5f;
		int32_t y = ImGui::GetCurrentWindow( )->Pos.y;

		ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( x, y ), ImVec2( x + 181, y + 22 ), ImColor( 24.0f / 255.0f, 31.0f / 255.0f, 44.0f / 255.0f, 1.0f ) );
		ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( x, y + 22 ), ImVec2( x + 181, y + 22.27f ), ImColor( 0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 1.0f ) );
		ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( x, y + 22.27f ), ImVec2( x + 181, y + 22.30f ), ImColor( 0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 0.65f ) );

		ImGui::PushFont( g_Globals.m_Fonts.m_UserIcon );
		ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 5, y + 2 ), ImColor( 71, 163, 255 ), _S( "a" ) );
		ImGui::PopFont( );

		ImGui::PushFont( g_Globals.m_Fonts.m_LogFont );
		ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 26, y + 2 ), ImColor( 255, 255, 255 ), _S( "Spectator list" ) );
		ImGui::PopFont( );

		int m_Last = 0;
		for ( auto& Spectator : vecSpectatorList )
		{
			ImGui::PushFont( g_Globals.m_Fonts.m_LogFont );
			ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 2, 23 + ( y + 16 * m_Last ) ), ImColor( 255, 255, 255, 255 ), Spectator.c_str( ) );
			ImGui::PopFont( );

			m_Last++;
		}
	}
	ImGui::DefaultEnd( );
}