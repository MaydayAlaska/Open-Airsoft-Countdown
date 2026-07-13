#pragma once

#include "Buzzer.h"
#include "Display.h"
#include "KeypadInput.h"
#include "NfcReader.h"
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
	void handleRunning(char key);

	bool isDigit(char key) const;
	uint32_t parseTimerInput() const;
	String formatDisarmPinMask() const;

	Display m_display;
	Storage m_storage;
	Timer m_timer;
	Users m_users;
	KeypadInput m_keypad;
	NfcReader m_nfcReader;
	Buzzer m_buzzer;

	Mode m_mode = Mode::EnterAdminPin;

	String m_adminPinInput;
	String m_timerInput;
	String m_disarmPinInput;

	uint8_t m_errorCount = 0;

	uint32_t m_lastDisplayedSeconds = 0xFFFFFFFF;
};