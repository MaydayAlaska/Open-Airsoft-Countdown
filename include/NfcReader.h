#pragma once

#include <Arduino.h>
#include <Adafruit_PN532.h>
#include <Wire.h>

class NfcReader
{
public:
	NfcReader();

	bool begin();
	void update();

	bool hasNewUid() const;
	String getLastUid() const;
	void clearNewUid();

private:
	static constexpr uint8_t I2cSdaPin = 1;
	static constexpr uint8_t I2cSclPin = 2;
	static constexpr uint8_t Pn532IrqPin = 10;
	// The Adafruit library requires a reset GPIO argument. On PN532 V3 modules
	// RSTO is a module output, so this GPIO is deliberately left unconnected.
	static constexpr uint8_t UnusedLibraryResetPin = 11;
	static constexpr uint32_t RestartDelayMs = 500;

	void startListening();
	void handleCardDetected();
	String uidToString(const uint8_t *uid, uint8_t uidLength) const;

	TwoWire m_wire;
	Adafruit_PN532 m_pn532;

	bool m_available = false;
	bool m_hasNewUid = false;
	bool m_readerDisabled = false;
	bool m_detectionStarted = false;
	int m_irqPrevious = HIGH;

	String m_lastUid;
	uint32_t m_lastCardReadMillis = 0;
};
