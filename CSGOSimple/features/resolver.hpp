#pragma once
#include "../valve_sdk/sdk.hpp"
#include "../valve_sdk/csgostructs.hpp"
class Resolver : public Singleton<Resolver>
{
public:
	void Resolve(C_BasePlayer* player);
};