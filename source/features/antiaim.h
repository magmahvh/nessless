#pragma once

#include "../singleton.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include <deque>
#include <algorithm>

class AntiAim : public Singleton<AntiAim> {
public:
	void slidewalk(CUserCmd* cmd);
};