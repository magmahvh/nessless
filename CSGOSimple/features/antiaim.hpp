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
	//void manage_local_fake_animstate();
	QAngle angle{};
	//matrix3x4_t fake_matrix[128];
	//bool is_fakematrix_setup = false;
};