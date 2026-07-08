#include "Application.h"

bool Application::begin()
{
	if (!m_display.begin())
	{
		return false;
	}

	return true;
}

void Application::update()
{
	m_display.update();
}