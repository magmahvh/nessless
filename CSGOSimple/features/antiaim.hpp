#pragma once

#include "..//options.hpp"
#include "..//valve_sdk/csgostructs.hpp"
#include "../lua/CLua.h"

class Antiaim : public Singleton<Antiaim>
{
public:
	int pitch = 0;
	int yaw = 0;
	int dsy = 0;
	void Run(CUserCmd* pCmd, bool& bSendPacket);
	QAngle angle{};
};