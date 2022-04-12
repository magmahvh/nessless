#pragma once
#include "../valve_sdk/sdk.hpp"
#include "../valve_sdk/csgostructs.hpp"

enum
{
	DSY_ORIGINAL,
	DSY_ZERO,
	DSY_FIRST,
	DSY_SECOND,
	DSY_LOW_FIRST,
	DSY_LOW_SECOND
};

class Resolver : public Singleton<Resolver>
{
private:
	float storedBrute[65];
public:
	void Run(ClientFrameStage_t stage);
};