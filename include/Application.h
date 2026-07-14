#pragma once

#include "BleManager.h"
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
		Stopped,
		Finished
	};

	void handleAdminPin(char key);
	void handleSetTimer(char key);
	void handleRunning(char key);
	void handleStopped(char key);

	void showPinErrorMessage(uint32_t remainingSeconds);
	void updatePinErrorMessage();
	void restoreDisplayAfterPinError();

	void handleBleCommand(const String &command);
	void sendBleStatus();
	void sendBleStatusIfChanged();
	void sendBleConfig();
	void sendBleUsers();

	bool applyBleConfig(const String &body);
	bool addBleUser(const String &body);
	bool updateBleUser(const String &body);
	String getCommandValue(const String &body, const String &key) const;
	bool parseBooleanValue(const String &value, bool &result) const;
	bool isUnsignedNumber(const String &value) const;
	bool isValidPin(const String &pin) const;
	bool isValidProtocolText(const String &value, uint8_t maxLength) const;
	bool isHexString(const String &value) const;

	bool isDigit(char key) const;
	uint32_t parseTimerInput() const;
	uint32_t parseTimerText(const String &input) const;
	const char *modeToString() const;

	Display m_display;
	Storage m_storage;
	Timer m_timer;
	Users m_users;
	KeypadInput m_keypad;
	NfcReader m_nfcReader;
	Buzzer m_buzzer;
	BleManager m_bleManager;

	Mode m_mode = Mode::EnterAdminPin;

	String m_adminPinInput;
	String m_timerInput;
	String m_disarmPinInput;

	uint8_t m_errorCount = 0;

	bool m_bleLoggedIn = false;

	bool m_pinErrorMessageActive = false;
	uint32_t m_pinErrorMessageStartedAt = 0;

	uint32_t m_lastDisplayedSeconds = 0xFFFFFFFF;

	bool m_hasLastBleStatus = false;
	Mode m_lastBleStatusMode = Mode::EnterAdminPin;
	uint32_t m_lastBleStatusRemainingSeconds = 0xFFFFFFFF;
	uint32_t m_lastBleStatusDurationSeconds = 0xFFFFFFFF;
	uint8_t m_lastBleStatusErrorCount = 0xFF;
	uint32_t m_lastBleStatusMaxErrorCount = 0xFFFFFFFF;
	bool m_lastBleStatusLocked = false;
	bool m_lastBleStatusLoggedIn = false;
};
