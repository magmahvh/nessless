/*
#pragma once

#include "..//options.hpp"
#include "..//valve_sdk/csgostructs.hpp"
#include "utils.hpp"

#include <deque>

class Logs : public Singleton<Logs>
{
public:
	void Create(std::string text);
	void Draw();
private:
	struct loginfo_t
	{
		loginfo_t(float log_time, std::string message)  //-V818
		{
			this->log_time = log_time;
			this->message = message; //-V820

			x = 6.0f;
			y = 0.0f;
		}

		float log_time;
		std::string message;
		float x, y;
	};

	std::deque <loginfo_t> logs;
};
*/