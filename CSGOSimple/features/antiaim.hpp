#pragma once

#include "..//options.hpp"
#include "..//valve_sdk/csgostructs.hpp"

class Antiaim : public Singleton<Antiaim>
{
public:
	void Run(CUserCmd* pCmd, bool& bSendPacket);
};