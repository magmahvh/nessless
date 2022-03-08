/*
#pragma once

#include "..//options.hpp"
#include "..//valve_sdk/csgostructs.hpp"
#include "utils.hpp"

#include <deque>

class Logs : public Singleton<Logs>
{
public:
	void Create(const char* text);
	void Draw();
private:
	struct loginfo_t
	{
		loginfo_t(float log_time, std::string message, const Color& color)  //-V818
		{
			this->log_time = log_time;
			this->message = message; //-V820
			this->color = color;

			x = 6.0f;
			y = 0.0f;
		}

		float log_time;
		std::string message;
		Color color;
		float x, y;
	};

	std::deque <loginfo_t> logs;
};
*/