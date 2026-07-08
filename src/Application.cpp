#include "Application.h"

bool Application::begin()
{
	if (!m_storage.begin())
	{
		return false;
	}

	if (!m_display.begin())
	{
		return false;
	}

	if (!m_timer.begin())
	{
		return false;
	}

	return true;
}

void Application::update()
{
	m_timer.update();
	m_display.update();
}