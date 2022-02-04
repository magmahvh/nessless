#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include "grenades.h"

// Пусть будет здесь
void IEngineTrace::TraceLine(const Vector& src, const Vector& dst, int mask, IHandleEntity* entity, int collision_group, CGameTrace* trace) {
	static auto trace_filter_simple = Utils::Scan("client.dll", "55 8B EC 83 E4 F0 83 EC 7C 56 52") + 0x3D;

	std::uintptr_t filter[4] = {
		*reinterpret_cast<std::uintptr_t*>(trace_filter_simple),
		reinterpret_cast<std::uintptr_t>(entity),
		collision_group,
		0
	};

	auto ray = Ray_t();

	ray.Init(src, dst);

	TraceRay(ray, mask, reinterpret_cast<CTraceFilter*>(&filter), trace);
}

void IEngineTrace::TraceHull(const Vector& src, const Vector& dst, const Vector& mins, const Vector& maxs, int mask, IHandleEntity* entity, int collision_group, CGameTrace* trace) {
	static auto trace_filter_simple = Utils::Scan("client.dll", "55 8B EC 83 E4 F0 83 EC 7C 56 52") + 0x3D;

	std::uintptr_t filter[4] = {
		*reinterpret_cast<std::uintptr_t*>(trace_filter_simple),
		reinterpret_cast<std::uintptr_t>(entity),
		collision_group,
		0
	};

	auto ray = Ray_t();

	ray.Init(src, dst, mins, maxs);

	TraceRay(ray, mask, reinterpret_cast<CTraceFilter*>(&filter), trace);
}

void c_grenade_prediction::on_create_move(CUserCmd* cmd) {
	m_data = {};

	if (g_LocalPlayer->IsDead() || !g_Options.grenades)
		return;

	const auto weapon = reinterpret_cast<C_BaseCombatWeapon*>(g_EntityList->GetClientEntityFromHandle(g_LocalPlayer->m_hActiveWeapon()));
	if (!weapon || !weapon->m_bPinPulled() && weapon->m_fThrowTime() == 0.f)
		return;

	const auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data
		|| weapon_data->type != 9)//weapon_type_grenade
		return;

	m_data.m_owner = g_LocalPlayer;
	m_data.m_index = weapon->m_Item().m_iItemDefinitionIndex();

	auto view_angles = cmd->viewangles;

	if (view_angles.pitch < -90.f) {
		view_angles.pitch += 360.f;
	}
	else if (view_angles.pitch > 90.f) {
		view_angles.pitch -= 360.f;
	}

	view_angles.pitch -= (90.f - std::fabsf(view_angles.pitch)) * 10.f / 90.f;

	auto direction = Vector();

	Math::AngleVectors(view_angles, direction);

	const auto throw_strength = std::clamp< float >(weapon->m_flThrowStrength(), 0.f, 1.f);
	const auto eye_pos = g_LocalPlayer->GetEyePos();
	const auto src = Vector(eye_pos.x, eye_pos.y, eye_pos.z + (throw_strength * 12.f - 12.f));

	auto trace = CGameTrace();

	g_EngineTrace->TraceHull(
		src, src + direction * 22.f, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f },
		MASK_SOLID | CONTENTS_CURRENT_90, g_LocalPlayer, COLLISION_GROUP_NONE, &trace
	);

	m_data.predict(
		trace.endpos - direction * 6.f,
		direction * (std::clamp< float >(
			weapon_data->throw_velocity * 0.9f, 15.f, 750.f
			) * (throw_strength * 0.7f + 0.3f)) + g_LocalPlayer->m_vecVelocity() * 1.25f,
		g_GlobalVars->curtime,
		0
	);
}

bool c_grenade_prediction::data_t::draw() const
{
	if (m_path.size() <= 1u || g_GlobalVars->curtime >= m_expire_time)
		return false;


	auto prev_screen = Vector();
	auto prev_on_screen = Math::WorldToScreen(std::get< Vector >(m_path.front()), prev_screen);

	for (auto i = 1u; i < m_path.size(); ++i) {
		auto cur_screen = Vector();
		const auto cur_on_screen = Math::WorldToScreen(std::get< Vector >(m_path.at(i)), cur_screen);

		if (prev_on_screen && cur_on_screen)
		{
			Render::Get().RenderLine(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, g_Options.grenades_color);
		}

		prev_screen = cur_screen;
		prev_on_screen = cur_on_screen;
	}

	Render::Get().RenderCircleFilled(prev_screen.x, prev_screen.y, 15, 100, Color(0, 0, 0, 255));
	Render::Get().RenderCircle(prev_screen.x, prev_screen.y, 15, 100, g_Options.grenades_color);
	Render::Get().RenderText("!", prev_screen.x, prev_screen.y - 10, 20.f, g_Options.grenades_color, true, false, g_pSecondFont);

	return true;
}

void c_grenade_prediction::grenade_warning(C_BasePlayer* entity)
{
	auto& predicted_nades = c_grenade_prediction::Get().get_list();

	static auto last_server_tick = g_ClientState->m_ClockDriftMgr.m_nServerTick;
	if (last_server_tick != g_ClientState->m_ClockDriftMgr.m_nServerTick) {
		predicted_nades.clear();

		last_server_tick = g_ClientState->m_ClockDriftMgr.m_nServerTick;
	}

	if (entity->IsDormant() || !g_Options.grenades)
		return;

	const auto client_class = entity->GetClientClass();
	if (!client_class
		|| client_class->m_ClassID != 114 && client_class->m_ClassID != 9)
		return;

	if (client_class->m_ClassID == 9) {
		const auto model = entity->GetModel();
		if (!model)
			return;

		const auto studio_model = g_MdlInfo->GetStudiomodel(model);
		if (!studio_model
			|| std::string_view(studio_model->szName).find("fraggrenade") == std::string::npos)
			return;
	}

	const auto handle = entity->GetRefEHandle().ToLong();

	if (entity->m_nExplodeEffectTickBegin()) {
		predicted_nades.erase(handle);

		return;
	}

	if (predicted_nades.find(handle) == predicted_nades.end()) {
		predicted_nades.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(handle),
			std::forward_as_tuple(
				reinterpret_cast<C_BaseCombatWeapon*>(entity)->m_hThrower(),
				client_class->m_ClassID == 114 ? WEAPON_MOLOTOV : WEAPON_HEGRENADE,
				entity->m_vecOrigin(), reinterpret_cast<C_BasePlayer*>(entity)->m_vecVelocity(),
				entity->get_creation_time(), TIME_TO_TICKS(reinterpret_cast<C_BasePlayer*>(entity)->m_flSimulationTime() - entity->get_creation_time())
			)
		);
	}

	if (predicted_nades.at(handle).draw())
		return;

	predicted_nades.erase(handle);
}
