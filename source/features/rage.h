#pragma once

#include "../valve_sdk/csgostructs.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include "../helpers/math.hpp"
#include <deque>
#include <algorithm>

struct target_selection_info
{
	target_selection_info(C_BasePlayer* player, const int& idx)
	{
		this->entity = player;
		this->i = idx;
	}
	C_BasePlayer* entity;
	int				i;
};

class Rage: public Singleton<Rage>{
public:
	void Aim(CUserCmd* cmd);
};