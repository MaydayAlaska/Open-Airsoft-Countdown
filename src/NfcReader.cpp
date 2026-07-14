#include "NfcReader.h"

NfcReader::NfcReader() :
	m_wire(1),
	m_pn532(Pn532IrqPin, Pn532ResetPin, &m_wire)
{
}

bool NfcReader::begin()
{
	Serial.println("Initializing PN532 NFC reader...");
	Serial.println("PN532 I2C pins: SDA=GPIO1, SCL=GPIO2");

	m_wire.begin(I2cSdaPin, I2cSclPin);

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
	String result;
	result.reserve(uidLength * 2);

	for (uint8_t i = 0; i < uidLength; i++)
	{
		if (uid[i] < 0x10)
		{
			result += '0';
		}

		result += String(uid[i], HEX);
	}

	result.toUpperCase();

	return result;
}
