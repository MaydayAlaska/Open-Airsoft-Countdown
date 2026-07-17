#include "NfcReader.h"

NfcReader::NfcReader() :
	m_wire(1),
	m_pn532(Pn532IrqPin, UnusedLibraryResetPin, &m_wire)
{
}

bool NfcReader::begin()
{
	Serial.println("Initializing PN532 NFC reader...");
	Serial.println("PN532 I2C pins: SDA=GPIO1, SCL=GPIO2");
	Serial.println("PN532 IRQ pin: GPIO10");
	Serial.println("PN532 RSTO: leave disconnected (it is a module output).");

	m_available = false;
	m_hasNewUid = false;
	m_readerDisabled = false;
	m_detectionStarted = false;
	m_irqPrevious = HIGH;
	m_lastUid = "";
	m_lastCardReadMillis = 0;

	pinMode(Pn532IrqPin, INPUT_PULLUP);
	m_wire.begin(I2cSdaPin, I2cSclPin);
	m_wire.setClock(100000);
	delay(50);

	if (!m_pn532.begin())
	{
		Serial.println("PN532 initialization failed. NFC disabled.");
		return true;
	}

	const uint32_t versionData = m_pn532.getFirmwareVersion();

	if (!versionData)
	{
		Serial.println("PN532 not found. NFC disabled.");
		return true;
	}

	Serial.print("PN532 found. Firmware version: ");
	Serial.print((versionData >> 16) & 0xFF, DEC);
	Serial.print(".");
	Serial.println((versionData >> 8) & 0xFF, DEC);
	Serial.print("PN532 IRQ initial state: ");
	Serial.println(digitalRead(Pn532IrqPin) == LOW ? "LOW" : "HIGH");

	m_available = true;
	startListening();
	Serial.println("PN532 ready.");
	return true;
}

void NfcReader::update()
{
	if (!m_available)
	{
		return;
	}

	if (m_readerDisabled)
	{
		if (millis() - m_lastCardReadMillis >= RestartDelayMs)
		{
			m_readerDisabled = false;
			startListening();
		}
		return;
	}

	if (!m_detectionStarted)
	{
		startListening();
		return;
	}

	const int irqCurrent = digitalRead(Pn532IrqPin);

	if (irqCurrent == LOW && m_irqPrevious == HIGH)
	{
		Serial.println("PN532 IRQ received.");
		handleCardDetected();
		return;
	}

	m_irqPrevious = irqCurrent;
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

void NfcReader::startListening()
{
	if (!m_available)
	{
		return;
	}

	m_irqPrevious = HIGH;
	m_detectionStarted = false;
	Serial.println("Starting passive detection for an ISO14443A tag...");

	const bool cardAlreadyPresent = m_pn532.startPassiveTargetIDDetection(
		PN532_MIFARE_ISO14443A
	);

	m_detectionStarted = true;

	if (cardAlreadyPresent)
	{
		Serial.println("NFC tag already present.");
		handleCardDetected();
		return;
	}

	Serial.println("PN532 armed. Waiting for IRQ.");
}

void NfcReader::handleCardDetected()
{
	uint8_t uid[7] = {0};
	uint8_t uidLength = 0;

	const bool success = m_pn532.readDetectedPassiveTargetID(uid, &uidLength);

	if (success && (uidLength == 4 || uidLength == 7))
	{
		m_lastUid = uidToString(uid, uidLength);
		m_hasNewUid = true;
		Serial.print("NFC UID: ");
		Serial.println(m_lastUid);
	}
	else
	{
		Serial.println("NFC tag read failed.");
	}

	m_detectionStarted = false;
	m_readerDisabled = true;
	m_lastCardReadMillis = millis();
	m_irqPrevious = HIGH;
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
