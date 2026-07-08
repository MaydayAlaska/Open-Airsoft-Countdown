#pragma once

#include "Display.h"
#include "Storage.h"
#include "Timer.h"

class Application
{
public:
	bool begin();
	void update();

private:
	Display m_display;
	Storage m_storage;
	Timer m_timer;
};