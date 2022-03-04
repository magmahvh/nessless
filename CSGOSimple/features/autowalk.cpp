#include "autowalk.hpp"
#include "../options.hpp"
#include "../helpers/math.hpp"
#include <algorithm>

void NormalizeInOut(Vector& vIn, Vector& vOut)
{
	float flLen = vIn.Length();
	if (flLen == 0) {
		vOut.Init(0, 0, 1);
		return;
	}
	flLen = 1 / flLen;
	vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);
}
void Autowalk::Run(QAngle& OrigAng, CUserCmd* cmd) {
	if (!g_Options.autowalk)
		return;

	bool walkbotBefore = false;

	QAngle viewangles;
	g_EngineClient->GetViewAngles(&viewangles);

	static int OldMouseX = OrigAng.yaw;
	int mousedx = OldMouseX - OrigAng.yaw;

	auto fDistanceToWall = [&](QAngle diff = QAngle(0, 0, 0))->float {
		auto tmpviewangles = viewangles + diff;
		trace_t tr;
		Ray_t ray;
		CTraceFilter filter;
		filter.pSkip = g_LocalPlayer;
		Vector begin = g_LocalPlayer->GetEyePos(), t, end;
		Math::AngleVectors(tmpviewangles, t);
		NormalizeInOut(t, end);
		end *= 8192.0f;
		end += begin;
		ray.Init(begin, end);
		g_EngineTrace->TraceRay(ray, 0x4600400B, &filter, &tr);
		return (begin - tr.endpos).Size();
	};

	static float old1, old2, old3;
	if (g_LocalPlayer->m_vecVelocity().Length() < 3)
	{
		viewangles.yaw += 2.0f;
	}
	float Distances = 100;
	if (fDistanceToWall() < Distances) // we are near to some wall
	{
		int turn = 5;
		float negativeDist = fDistanceToWall(QAngle(0, -1, 0)), positiveDist = fDistanceToWall(QAngle(0, 1, 0));
		if (abs(negativeDist - positiveDist) < 1.0f)
		{
			viewangles.yaw += turn;
		}
		else
		{
			viewangles.yaw += positiveDist < negativeDist ? -1 : 1;
		}
	}

	while (viewangles.yaw > 180.0f)
		viewangles.yaw -= 360.0f;
	while (viewangles.yaw < -180.0f)
		viewangles.yaw += 360.0f;

	g_EngineClient->SetViewAngles(&viewangles);

	if (!walkbotBefore)
	{
		cmd->forwardmove = 450.f;
		walkbotBefore = true;
	}

	else if (walkbotBefore)
	{
		walkbotBefore = false;
		cmd->forwardmove = 450.f;
	}
}