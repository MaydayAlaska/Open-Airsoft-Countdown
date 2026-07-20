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
	Serial.println("PN532 IRQ pin: GPIO10");
	Serial.println("PN532 RESET pin: GPIO11");

	m_available = false;
	m_enabled = false;
	m_hasNewUid = false;
	m_readerDisabled = false;
	m_detectionStarted = false;
	m_configurationPending = false;
	m_waitingForRecoveryRetry = false;
	m_uidReleasePending = false;
	m_lastUid = "";
	m_lastResetMillis = 0;
	m_lastConfigurationAttemptMillis = 0;
	m_detectionStartedMillis = 0;
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
	resetForRearm();
	Serial.println("PN532 ready. NFC listening disabled until countdown starts.");
	return true;
}

void NfcReader::setEnabled(bool enabled)
{
	if (!m_available || m_enabled == enabled)
	{
		return;
	}

	m_enabled = enabled;
	m_hasNewUid = false;
	m_readerDisabled = false;
	m_detectionStarted = false;

	if (!m_enabled)
	{
		Serial.println("NFC listening disabled.");
		resetForRearm();
		return;
	}

	Serial.println("NFC listening enabled for active countdown.");

	if (!m_configurationPending)
	{
		resetForRearm();
	}
}

void NfcReader::resetSession()
{
	m_hasNewUid = false;
	m_lastUid = "";
	m_uidReleasePending = false;
}

void NfcReader::update()
{
	if (!m_available || !m_enabled)
	{
		return;
	}

	if (m_configurationPending)
	{
		configureAndStartListening();
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

	if (digitalRead(Pn532IrqPin) == LOW)
	{
		Serial.println("PN532 IRQ received.");
		handleCardDetected();
		return;
	}

	if (
		m_uidReleasePending &&
		millis() - m_detectionStartedMillis >= UidReleaseDelayMs
	)
	{
		Serial.println("NFC field remained clear. Previous UID can be presented again.");
		m_lastUid = "";
		m_uidReleasePending = false;
	}

	if (millis() - m_detectionStartedMillis >= DetectionWatchdogMs)
	{
		Serial.println("PN532 detection watchdog: performing a slow rearm.");
		resetForRearm();
	}
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

void NfcReader::resetForRearm()
{
	m_readerDisabled = false;
	m_detectionStarted = false;
	m_detectionStartedMillis = 0;
	m_configurationPending = true;
	m_waitingForRecoveryRetry = false;
	m_uidReleasePending = false;

	m_pn532.reset();
	m_lastResetMillis = millis();
}

void NfcReader::configureAndStartListening()
{
	const uint32_t currentMillis = millis();

	if (currentMillis - m_lastResetMillis < ResetSettleMs)
	{
		return;
	}

	if (
		m_waitingForRecoveryRetry &&
		currentMillis - m_lastConfigurationAttemptMillis < RecoveryRetryMs
	)
	{
		return;
	}

	m_lastConfigurationAttemptMillis = currentMillis;

	if (!m_pn532.SAMConfig())
	{
		Serial.println("PN532 reconfiguration failed. Retrying in 5 seconds.");
		m_waitingForRecoveryRetry = true;
		m_pn532.reset();
		m_lastResetMillis = millis();
		return;
	}

	m_configurationPending = false;
	m_waitingForRecoveryRetry = false;
	Serial.println("PN532 reconfigured after reset.");
	startListening();
}

void NfcReader::startListening()
{
	if (!m_available || !m_enabled)
	{
		return;
	}

	m_detectionStarted = false;
	m_uidReleasePending = false;
	Serial.println("Starting passive detection for an ISO14443A tag...");

	const bool cardAlreadyPresent = m_pn532.startPassiveTargetIDDetection(
		PN532_MIFARE_ISO14443A
	);

	m_detectionStarted = true;
	m_detectionStartedMillis = millis();

	if (cardAlreadyPresent)
	{
		Serial.println("NFC tag already present.");
		handleCardDetected();
		return;
	}

	m_uidReleasePending = m_lastUid.length() > 0;

	Serial.println("PN532 armed. Waiting for IRQ.");
}

void NfcReader::handleCardDetected()
{
	uint8_t uid[7] = {0};
	uint8_t uidLength = 0;

	const bool success = m_pn532.readDetectedPassiveTargetID(uid, &uidLength);

	if (success && (uidLength == 4 || uidLength == 7))
	{
		const String detectedUid = uidToString(uid, uidLength);

		if (detectedUid != m_lastUid)
		{
			m_lastUid = detectedUid;
			m_hasNewUid = true;
			Serial.print("NFC UID: ");
			Serial.println(m_lastUid);
		}
		else
		{
			Serial.println("NFC tag still present. Duplicate ignored.");
		}
	}
	else
	{
		Serial.println("NFC tag read failed.");
	}

	m_detectionStarted = false;
	m_detectionStartedMillis = 0;
	m_uidReleasePending = false;
	m_readerDisabled = true;
	m_lastCardReadMillis = millis();
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
