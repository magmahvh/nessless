#pragma once
#include "../valve_sdk/sdk.hpp"
#include "../valve_sdk/csgostructs.hpp"
class Autowalk : public Singleton<Autowalk>
{
public:
	void Run(QAngle& OrigAng, CUserCmd* cmd);
};