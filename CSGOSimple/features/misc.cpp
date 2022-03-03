#include "misc.hpp"

#include "../helpers/math.hpp"

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
		/* vector for the inverse angles */
		QAngle inverseAngles;
		g_EngineClient->GetViewAngles(&inverseAngles);

		/* inverse angles by 180 */
		inverseAngles.pitch *= -1.f, inverseAngles.yaw += 180.f;

		/* vector for direction */
		Vector direction;
		Math::AngleVectors(inverseAngles, direction);

		/* ray, trace & filters */
		Ray_t ray;
		trace_t trace;
		CTraceFilter filter;

		/* dont trace local player */
		filter.pSkip = g_LocalPlayer;

		/* create ray */
		ray.Init(g_LocalPlayer->GetEyePos(), g_LocalPlayer->GetEyePos() + (direction * ideal_distance));

		/* trace ray */
		g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);

		/* return the ideal distance */
		return (ideal_distance * trace.fraction) - 10.f;
	};

	QAngle angles;
	g_EngineClient->GetViewAngles(&angles);
	angles.roll = GetCorrectDistance(g_Options.misc_thirdperson_dist); // 150 is better distance
	g_Input->m_vecCameraOffset = Vector(angles.pitch, angles.yaw, angles.roll);
}