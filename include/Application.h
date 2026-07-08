#pragma once

#include "Buzzer.h"
#include "Display.h"
#include "KeypadInput.h"
#include "Storage.h"
#include "Timer.h"
#include "Users.h"

class Application
{
public:
	bool begin();
	void update();

private:
	enum class Mode
	{
		EnterAdminPin,
		SetTimer,
		Running,
		Finished
	};

	void handleAdminPin(char key);
	void handleSetTimer(char key);
	void handleRunning();

	bool isDigit(char key) const;
	uint32_t parseTimerInput() const;

	Display m_display;
	Storage m_storage;
	Timer m_timer;
	Users m_users;
	KeypadInput m_keypad;
	Buzzer m_buzzer;

	Mode m_mode = Mode::EnterAdminPin;

	String m_adminPinInput;
	String m_timerInput;

	uint8_t m_errorCount = 0;

	uint32_t m_lastDisplayedSeconds = 0xFFFFFFFF;
};