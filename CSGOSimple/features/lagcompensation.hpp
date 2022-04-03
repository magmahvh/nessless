#pragma once
#include "..\valve_sdk\csgostructs.hpp"

class CLagCompensation : public Singleton<CLagCompensation>
{
	public:
		float LagFix();
};