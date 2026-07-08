#include "Timer.h"

#include <Arduino.h>

bool Timer::begin()
{
	Serial.println("Initializing timer...");

	Serial.println("Timer initialized.");

	return true;
}

void Timer::update()
{
}