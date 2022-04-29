#include <algorithm>

#include "visuals.hpp"

#include "../options.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "ragebot.hpp"


RECT GetBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}

Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() {
	DeleteCriticalSection(&cs);
}

//--------------------------------------------------------------------------------
void Visuals::Render() {
}
//--------------------------------------------------------------------------------
int flPlayerAlpha[64];
int flAlphaFade = 5.f;

bool Visuals::Player::Begin(C_BasePlayer* pl)
{
	if (!pl->IsAlive())
		return false;

	if (pl->IsDormant() && flPlayerAlpha[pl->EntIndex()] > 120)
	{
		flPlayerAlpha[pl->EntIndex()] -= flAlphaFade;
	}
	else if (flPlayerAlpha[pl->EntIndex()] < 255 && !(pl->IsDormant()))
	{
		flPlayerAlpha[pl->EntIndex()] += flAlphaFade;
	}
	ctx.pl = pl;
	ctx.is_enemy = g_LocalPlayer->m_iTeamNum() != pl->m_iTeamNum();
	ctx.is_visible = g_LocalPlayer->CanSeePlayer(pl, HITBOX_CHEST);

	if (!ctx.is_enemy)
		return false;
	if (!ctx.is_visible && !g_Options.esp_player_boxesOccluded)
		return false;
	//ctx.clr = ctx.is_enemy ? (ctx.is_visible ? g_Options.color_esp_enemy_visible : g_Options.color_esp_enemy_occluded) : (ctx.is_visible ? g_Options.color_esp_ally_visible : g_Options.color_esp_ally_occluded);

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	if (!Math::WorldToScreen(head, ctx.head_pos) ||
		!Math::WorldToScreen(origin, ctx.feet_pos))
		return false;

	ctx.bbox = GetBBox(pl);

	std::swap(ctx.bbox.top, ctx.bbox.bottom);

	return !(!ctx.bbox.left || !ctx.bbox.top || !ctx.bbox.right || !ctx.bbox.bottom);
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderBox(C_BaseEntity* pl) {
	if (!ctx.is_visible && g_Options.esp_player_boxesOccluded) {
		Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, Color(g_Options.color_esp_enemy_occluded[0], g_Options.color_esp_enemy_occluded[1], g_Options.color_esp_enemy_occluded[2], flPlayerAlpha[pl->EntIndex()]), 1);
		Render::Get().RenderBoxByType(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, Color(0, 0, 0, flPlayerAlpha[pl->EntIndex()]), 1);
		Render::Get().RenderBoxByType(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, Color(0, 0, 0, flPlayerAlpha[pl->EntIndex()]), 1);
	}
	if (ctx.is_visible) {
		Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, Color(g_Options.color_esp_enemy_visible[0], g_Options.color_esp_enemy_visible[1], g_Options.color_esp_enemy_visible[2], flPlayerAlpha[pl->EntIndex()]), 1);
		Render::Get().RenderBoxByType(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, Color(0, 0, 0, flPlayerAlpha[pl->EntIndex()]), 1);
		Render::Get().RenderBoxByType(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, Color(0, 0, 0, flPlayerAlpha[pl->EntIndex()]), 1);
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderName(C_BaseEntity* pl)
{
	player_info_t info = ctx.pl->GetPlayerInfo();

	auto sz = g_pDefaultFont->CalcTextSizeA(12.f, FLT_MAX, 0.0f, info.szName);

//	Render::Get().RenderText(info.szName, ctx.feet_pos.x - sz.x / 2, ctx.head_pos.y - sz.y, 12.f, Color::White);
	Render::Get().RenderText(info.szName, ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left - sz.x) / 2, (ctx.bbox.top - sz.y - 1), 12.f, Color(255, 255, 255, flPlayerAlpha[pl->EntIndex()]), false, true, g_EspFont);

}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderHealth(C_BaseEntity* pl)
{
	auto  hp = ctx.pl->m_iHealth();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	//float off = (box_h / 6.f) + 5;
	float off = 8;

	int height = (box_h * hp) / 100;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;
	std::string text = std::to_string(hp);

	Render::Get().RenderBox(x, y - 1, x + w, y + h + 1, Color(0, 0, 0, flPlayerAlpha[pl->EntIndex()]));
	Render::Get().RenderBox(x + 1, y + h - height, x + w - 1, y + h, Color(0, 255, 0, flPlayerAlpha[pl->EntIndex()]));
	Render::Get().RenderText(text, x - 20, y + h - height, 12.f, Color(255, 255, 255, flPlayerAlpha[pl->EntIndex()]), false, true, g_EspFont);

}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderArmour(C_BaseEntity* pl)
{
	auto  armour = ctx.pl->m_ArmorValue();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	//float off = (box_h / 6.f) + 5;
	float off = 4;

	int height = (((box_h * armour) / 100));

	int x = ctx.bbox.right + off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;

	Render::Get().RenderBox(x, y, x + w, y + h, Color::Black, 1.f, true);
	Render::Get().RenderBox(x + 1, y + 1, x + w - 1, y + height - 2, Color(0, 50, 255, flPlayerAlpha[pl->EntIndex()]), 1.f, true);
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderWeaponName(C_BaseEntity* pl)
{
	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon) return;
	if (!weapon->GetCSWeaponData()) return;

	auto text = weapon->GetCSWeaponData()->szWeaponName + 7;
	auto sz = g_pDefaultFont->CalcTextSizeA(12.f, FLT_MAX, 0.0f, text);
	//Render::Get().RenderText(text, ctx.feet_pos.x, ctx.feet_pos.y, 12.f, Color::White, true,g_pDefaultFont);
	Render::Get().RenderText(text, ImVec2(ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left - sz.x) / 2, ctx.bbox.bottom + 1), 12.f, Color(255, 255, 255, flPlayerAlpha[pl->EntIndex()]), false, true, g_EspFont);

}
//--------------------------------------------------------------------------------
void Visuals::RenderWeapon(C_BaseCombatWeapon* ent)
{
	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	// We don't want to Render weapons that are being held
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	//Render::Get().RenderBox(bbox, g_Options.color_esp_weapons);


	auto name = clean_item_name(ent->GetClientClass()->m_pNetworkName);

	auto sz = g_pDefaultFont->CalcTextSizeA(12.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;


	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 12.f, Color::White, false, true, g_EspFont);
}
//--------------------------------------------------------------------------------
void Visuals::RenderDefuseKit(C_BaseEntity* ent)
{
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Options.color_esp_defuse);

	auto name = "Defuse Kit";
	auto sz = g_pDefaultFont->CalcTextSizeA(12.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;
	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 12.f, Color::White, false, true, g_EspFont);
}
//--------------------------------------------------------------------------------
void Visuals::RenderPlantedC4(C_BaseEntity* ent)
{
	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;


	Render::Get().RenderBox(bbox, g_Options.color_esp_c4);


	int bombTimer = std::ceil(ent->m_flC4Blow() - g_GlobalVars->curtime);
	std::string timer = std::to_string(bombTimer);

	auto name = (bombTimer < 0.f) ? "Bomb" : timer;
	auto sz = g_pDefaultFont->CalcTextSizeA(12.f, FLT_MAX, 0.0f, name.c_str());
	int w = bbox.right - bbox.left;

	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 12.f, g_Options.color_esp_c4, false, true, g_EspFont);
}
//--------------------------------------------------------------------------------
void Visuals::RenderItemEsp(C_BaseEntity* ent)
{
	std::string itemstr = "Undefined";
	const model_t * itemModel = ent->GetModel();
	if (!itemModel)
		return;
	studiohdr_t * hdr = g_MdlInfo->GetStudiomodel(itemModel);
	if (!hdr)
		return;
	itemstr = hdr->szName;
	if (ent->GetClientClass()->m_ClassID == ClassId_CBumpMine)
		itemstr = "";
	else if (itemstr.find("case_pistol") != std::string::npos)
		itemstr = "Pistol Case";
	else if (itemstr.find("case_light_weapon") != std::string::npos)
		itemstr = "Light Case";
	else if (itemstr.find("case_heavy_weapon") != std::string::npos)
		itemstr = "Heavy Case";
	else if (itemstr.find("case_explosive") != std::string::npos)
		itemstr = "Explosive Case";
	else if (itemstr.find("case_tools") != std::string::npos)
		itemstr = "Tools Case";
	else if (itemstr.find("random") != std::string::npos)
		itemstr = "Airdrop";
	else if (itemstr.find("dz_armor_helmet") != std::string::npos)
		itemstr = "Full Armor";
	else if (itemstr.find("dz_helmet") != std::string::npos)
		itemstr = "Helmet";
	else if (itemstr.find("dz_armor") != std::string::npos)
		itemstr = "Armor";
	else if (itemstr.find("upgrade_tablet") != std::string::npos)
		itemstr = "Tablet Upgrade";
	else if (itemstr.find("briefcase") != std::string::npos)
		itemstr = "Briefcase";
	else if (itemstr.find("parachutepack") != std::string::npos)
		itemstr = "Parachute";
	else if (itemstr.find("dufflebag") != std::string::npos)
		itemstr = "Cash Dufflebag";
	else if (itemstr.find("ammobox") != std::string::npos)
		itemstr = "Ammobox";
	else if (itemstr.find("dronegun") != std::string::npos)
		itemstr = "Turrel";
	else if (itemstr.find("exojump") != std::string::npos)
		itemstr = "Exojump";
	else if (itemstr.find("healthshot") != std::string::npos)
		itemstr = "Healthshot";
	else {
		/*May be you will search some missing items..*/
		/*static std::vector<std::string> unk_loot;
		if (std::find(unk_loot.begin(), unk_loot.end(), itemstr) == unk_loot.end()) {
			Utils::ConsolePrint(itemstr.c_str());
			unk_loot.push_back(itemstr);
		}*/
		return;
	}
	
	auto bbox = GetBBox(ent);
	if (bbox.right == 0 || bbox.bottom == 0)
		return;
	auto sz = g_pDefaultFont->CalcTextSizeA(12.f, FLT_MAX, 0.0f, itemstr.c_str());
	int w = bbox.right - bbox.left;


	//Render::Get().RenderBox(bbox, g_Options.color_esp_item);
	Render::Get().RenderText(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 12.f, g_Options.color_esp_item, false, true, g_EspFont);
}
void Visuals::RenderMultipoints() {
	for (auto i = 1; i <= g_GlobalVars->maxClients; i++)
	{
		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

		if (!player || !player->IsAlive() || !player->IsPlayer() || player->m_bGunGameImmunity())
			continue;

		if (!player->IsEnemy())
			continue;

		for (const auto hitbox : g_Ragebot->hitboxes_active)
		{
			for (auto hitboxPos : g_Ragebot->GetMultipoints(player, hitbox, nullptr)) {
				Vector hbPos = hitboxPos.first;
				Vector screenPos = {};

				Math::WorldToScreen(hbPos, screenPos);

				Render::Get().RenderCircleFilled(screenPos.x, screenPos.y, 3, 6, Color(255, 255, 255, 255));
			}
		}
	}
}
void Visuals::AddToDrawList() {
	for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) {
		auto entity = C_BaseEntity::GetEntityByIndex(i);

		if (!entity)
			continue;
		
		if (entity == g_LocalPlayer && !g_Input->bCameraInThirdPerson)
			continue;

		if (i <= g_GlobalVars->maxClients) {
			auto player = Player();
			if (player.Begin((C_BasePlayer*)entity)) {
				if (g_Options.esp_player_boxes)     player.RenderBox(entity);
				if (g_Options.esp_player_weapons)   player.RenderWeaponName(entity);
				if (g_Options.esp_player_names)     player.RenderName(entity);
				if (g_Options.esp_player_health)    player.RenderHealth(entity);
				if (g_Options.esp_player_armour)    player.RenderArmour(entity);
			}
		}
		else if (g_Options.esp_dropped_weapons && entity->IsWeapon())
			RenderWeapon(static_cast<C_BaseCombatWeapon*>(entity));
		else if (g_Options.esp_dropped_weapons && entity->IsDefuseKit())
			RenderDefuseKit(entity);
		else if (entity->IsPlantedC4() && g_Options.esp_planted_c4)
			RenderPlantedC4(entity);
		else if (entity->IsLoot() && g_Options.esp_items)
			RenderItemEsp(entity);

		if (g_Options.draw_multipoints)
			RenderMultipoints();
	}
}
