#include "lagcompensation.hpp"
#include "..\helpers\math.hpp"
#include "..\valve_sdk\csgostructs.hpp"

template<class T, class U>
T clamp2(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}

#define max(a,b) (((a) > (b)) ? (a) : (b))

float CLagCompensation::LagFix()
{
	static auto cl_ud_rate = g_CVar->FindVar("cl_updaterate");
	static auto min_ud_rate = g_CVar->FindVar("sv_minupdaterate");
	static auto max_ud_rate = g_CVar->FindVar("sv_maxupdaterate");

	int ud_rate = 64;

	if (cl_ud_rate)
		ud_rate = cl_ud_rate->GetInt();

	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->GetInt();

	float ratio = 1.f;
	static auto cl_interp_ratio = g_CVar->FindVar("cl_interp_ratio");

	if (cl_interp_ratio)
		ratio = cl_interp_ratio->GetFloat(); 

	static auto cl_interp = g_CVar->FindVar("cl_interp");
	static auto c_min_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
	static auto c_max_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");

	float lerp = g_GlobalVars->interval_per_tick;

	if (cl_interp)
		lerp = cl_interp->GetFloat();

	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
		ratio = clamp2(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

	return max(lerp, ratio / ud_rate);
}