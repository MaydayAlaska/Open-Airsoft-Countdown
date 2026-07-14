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
	static constexpr uint8_t Pn532ResetPin = 11;

	static constexpr uint16_t ReadTimeoutMs = 20;
	static constexpr uint16_t SameCardCooldownMs = 1000;

	String uidToString(const uint8_t *uid, uint8_t uidLength) const;

	TwoWire m_wire;
	Adafruit_PN532 m_pn532;

	bool m_available = false;
	bool m_hasNewUid = false;

	String m_lastUid;
	uint32_t m_lastReadMillis = 0;
};
