#include "../singleton.hpp"
#include "../valve_sdk/csgostructs.hpp"


#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)

#define M_RADPI 57.295779513082f

#pragma once
class BunnyHop : public Singleton<BunnyHop>
{
public:
	void OnCreateMove(CUserCmd* cmd);
	void AutoStrafe(CUserCmd* cmd, QAngle va);
};