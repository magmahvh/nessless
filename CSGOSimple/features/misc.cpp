#include "misc.hpp"

#include "../helpers/math.hpp"
#include <algorithm>
#include "../helpers/keybinds.hpp"

void Misc::ThirdPerson() {
	if (!g_LocalPlayer)
		return;

	g_Input->bCameraInThirdPerson = g_Options.misc_thirdperson && KeyList::Get().thirdperson && g_LocalPlayer->IsAlive();
	if (!g_Input->bCameraInThirdPerson)
		return;

	QAngle angles;
	g_EngineClient->GetViewAngles(&angles);

	QAngle backward(angles.pitch, angles.yaw + 180.f, angles.roll);
	backward.Normalize();

	Vector range;
	Math::angle2vectors(backward, range);
	range *= 8192.f;

	const auto start = g_LocalPlayer->GetEyePos();

	CTraceFilter filter;
	filter.pSkip = g_LocalPlayer;

	Ray_t ray;
	ray.Init(start, start + range);

	trace_t tr;
	g_EngineTrace->TraceRay(ray, MASK_SHOT_HULL, &filter, &tr);

	angles.roll = std::min<int>(start.DistTo(tr.endpos), 150); // 150 is better distance

	g_Input->vecCameraOffset = angles.getVector();
}

