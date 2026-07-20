#pragma once

#include <Arduino.h>
#include <Adafruit_PN532.h>
#include <Wire.h>

class NfcReader
{
public:
	NfcReader();

	bool begin();
	void setEnabled(bool enabled);
	void resetSession();
	void update();

	bool hasNewUid() const;
	String getLastUid() const;
	void clearNewUid();

private:
	static constexpr uint8_t I2cSdaPin = 1;
	static constexpr uint8_t I2cSclPin = 2;
	static constexpr uint8_t Pn532IrqPin = 10;
	static constexpr uint8_t Pn532ResetPin = 11;
	static constexpr uint32_t ResetSettleMs = 10;
	static constexpr uint32_t RecoveryRetryMs = 5000;
	static constexpr uint32_t DetectionWatchdogMs = 30000;
	static constexpr uint32_t UidReleaseDelayMs = 250;
	static constexpr uint32_t RestartDelayMs = 2000;

	void resetForRearm();
	void configureAndStartListening();
	void startListening();
	void handleCardDetected();
	String uidToString(const uint8_t *uid, uint8_t uidLength) const;

	TwoWire m_wire;
	Adafruit_PN532 m_pn532;

	bool m_available = false;
	bool m_enabled = false;
	bool m_hasNewUid = false;
	bool m_readerDisabled = false;
	bool m_detectionStarted = false;
	bool m_configurationPending = false;
	bool m_waitingForRecoveryRetry = false;
	bool m_uidReleasePending = false;

	String m_lastUid;
	uint32_t m_lastResetMillis = 0;
	uint32_t m_lastConfigurationAttemptMillis = 0;
	uint32_t m_detectionStartedMillis = 0;
	uint32_t m_lastCardReadMillis = 0;
};
