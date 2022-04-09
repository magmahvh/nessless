#include "misc.hpp"

#include "../helpers/math.hpp"
#include <algorithm>
#include "../helpers/keybinds.hpp"
/*
void Thirdperson_Init(bool fakeducking, float progress) {

	static float current_fraction = 0.0f;

	auto distance = ((float)100) * progress;

	QAngle angles;
	QAngle inverse_angles;
	g_EngineClient->GetViewAngles(&angles);
	g_EngineClient->GetViewAngles(&inverse_angles);


	inverse_angles.roll = distance;

	Vector forward, right, up;

	Math::AngleVectors(inverse_angles, forward, right, up);

	auto eye_pos = fakeducking ? g_LocalPlayer->abs_origin() + g_GameMovement->GetPlayerViewOffset(false) : g_LocalPlayer->abs_origin() + g_LocalPlayer->m_vecViewOffset();

	auto offset = eye_pos + forward * -distance + right + up;

	CTraceFilterWorldOnly filter;
	trace_t tr;
	g_EngineTrace->TraceRay(Ray_t(eye_pos, offset, Vector(-16.0f, -16.0f, -16.0f), Vector(16.0f, 16.0f, 16.0f)), 131083, &filter, &tr);

	if (current_fraction > tr.fraction)

		current_fraction = tr.fraction;

	else if (current_fraction > 0.9999f)

		current_fraction = 1.0f;

	current_fraction = Math::Interpolate(current_fraction, tr.fraction, g_GlobalVars->frametime * 10.0f);
	angles.roll = distance * current_fraction;
}
*/

void Misc::ThirdPerson() {
/*

	static float progress;
	static bool in_transition;
	auto in_thirdperson = KeyList::Get().thirdperson;


	if (g_LocalPlayer->IsAlive() && in_thirdperson)
	{
		in_transition = false;
		if (!g_Input->m_fCameraInThirdPerson)
		{
			g_Input->m_fCameraInThirdPerson = true;
		}
	}
	else
	{
		progress -= g_GlobalVars->frametime * 8.f + (progress / 100);
		progress = std::clamp(progress, 0.f, 1.f);
		if (!progress)
			g_Input->m_fCameraInThirdPerson = false;
		else
		{
			in_transition = true;
			g_Input->m_fCameraInThirdPerson = true;
		}
	}
	if (g_Input->m_fCameraInThirdPerson && !in_transition)
	{
		progress += g_GlobalVars->frametime * 8.f + (progress / 100);
		progress = std::clamp(progress, 0.f, 1.f);
	}
	Thirdperson_Init(false, progress);
	*/
}

