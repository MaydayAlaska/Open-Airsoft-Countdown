#include "NfcReader.h"

#include <Wire.h>

NfcReader::NfcReader() :
	m_pn532(Pn532IrqPin, Pn532ResetPin, &Wire)
{
}

bool NfcReader::begin()
{
	Serial.println("Initializing PN532 NFC reader...");

	Wire.begin(I2cSdaPin, I2cSclPin);

	m_pn532.begin();

	const uint32_t versionData = m_pn532.getFirmwareVersion();

	if (!versionData)
	{
		Serial.println("PN532 not found. NFC disabled.");
		m_available = false;
		return true;
	}

	m_pn532.SAMConfig();

	Serial.print("PN532 found. Firmware version: ");
	Serial.print((versionData >> 16) & 0xFF, DEC);
	Serial.print(".");
	Serial.println((versionData >> 8) & 0xFF, DEC);

	m_available = true;

	return true;
}

void NfcReader::update()
{
	if (!m_available)
	{
		return;
	}

	const uint32_t currentMillis = millis();

	if (currentMillis - m_lastReadMillis < SameCardCooldownMs)
	{
		return;
	}

	uint8_t uid[7];
	uint8_t uidLength = 0;

	const bool cardFound = m_pn532.readPassiveTargetID(
		PN532_MIFARE_ISO14443A,
		uid,
		&uidLength,
		ReadTimeoutMs
	);

	if (!cardFound)
	{
		return;
	}

	m_lastUid = uidToString(uid, uidLength);
	m_hasNewUid = true;
	m_lastReadMillis = currentMillis;

	Serial.print("NFC UID: ");
	Serial.println(m_lastUid);
}

bool NfcReader::hasNewUid() const
{
	return m_hasNewUid;
}

String NfcReader::getLastUid() const
{
	return m_lastUid;
}

void NfcReader::clearNewUid()
{
	m_hasNewUid = false;
}

String NfcReader::uidToString(const uint8_t *uid, uint8_t uidLength) const
{
	const char hexChars[] = "0123456789ABCDEF";

	String result;
	result.reserve(uidLength * 2);

	for (uint8_t i = 0; i < uidLength; i++)
	{
		result += hexChars[(uid[i] >> 4) & 0x0F];
		result += hexChars[uid[i] & 0x0F];
	}

	return result;
}