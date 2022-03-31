#include "misc.hpp"

#include "../helpers/math.hpp"
/*
void Misc::ThirdPerson() {
	if (!g_LocalPlayer)
		return;

	static size_t lastTime = 0;

	if (GetAsyncKeyState(g_Options.misc_thirdperson_bind))
	{
		if (GetTickCount64() > lastTime) {
			g_Options.misc_thirdperson = !g_Options.misc_thirdperson;

			lastTime = GetTickCount64() + 650;
		}
	}

	if (g_Options.misc_thirdperson && g_LocalPlayer->IsAlive())
		g_Input->m_fCameraInThirdPerson = true;
	else
		g_Input->m_fCameraInThirdPerson = false;

	auto GetCorrectDistance = [](float ideal_distance) -> float
	{
		
		QAngle inverseAngles;
		g_EngineClient->GetViewAngles(&inverseAngles);

	
		inverseAngles.pitch *= -1.f, inverseAngles.yaw += 180.f;

	
		Vector direction;
		Math::AngleVectors(inverseAngles, direction);


		Ray_t ray;
		trace_t trace;
		CTraceFilter filter;

	
		filter.pSkip = g_LocalPlayer;

	
		ray.Init(g_LocalPlayer->GetEyePos(), g_LocalPlayer->GetEyePos() + (direction * ideal_distance));

		
		g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);

	
		return (ideal_distance * trace.fraction) - 10.f;
	};

	QAngle angles;
	g_EngineClient->GetViewAngles(&angles);
	angles.roll = GetCorrectDistance(g_Options.misc_thirdperson_dist); // 150 is better distance
	g_Input->m_vecCameraOffset = Vector(angles.pitch, angles.yaw, angles.roll);
}
*/