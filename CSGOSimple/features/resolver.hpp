#pragma once
#include "../valve_sdk/sdk.hpp"
#include "../valve_sdk/csgostructs.hpp"
class Resolver : public Singleton<Resolver>
{
private:
	float storedBrute[65];
public:
	void Run(ClientFrameStage_t stage);
};