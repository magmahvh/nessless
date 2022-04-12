#pragma once

#include "..//options.hpp"
#include "..//valve_sdk/csgostructs.hpp"



#define NUM_OF_TICKS 15

struct StoredData
{
	float simtime;
	Vector hitboxPos;
};

class TimeWarp : public Singleton<TimeWarp>
{
	int nLatestTick;
	StoredData TimeWarpData[64][NUM_OF_TICKS];
public:
	void CreateMove(CUserCmd* cmd);
};