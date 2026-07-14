#include "BleManager.h"

#include <NimBLEDevice.h>

namespace
{
	constexpr uint16_t PreferredMtu = 185;

	BleManager *ActiveBleManager = nullptr;

	class BleServerCallbacks : public NimBLEServerCallbacks
	{
	public:
		void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override
		{
			(void)server;

			Serial.print("BLE client connected: ");
			Serial.println(connInfo.getAddress().toString().c_str());
		}

		void onDisconnect(NimBLEServer *server, NimBLEConnInfo &connInfo, int reason) override
		{
			(void)server;
			(void)reason;

			Serial.print("BLE client disconnected: ");
			Serial.println(connInfo.getAddress().toString().c_str());

			NimBLEDevice::startAdvertising();
			Serial.println("BLE advertising restarted.");
		}
	};

	class BleCommandCallbacks : public NimBLECharacteristicCallbacks
	{
	public:
		void onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo &connInfo) override
		{
			(void)connInfo;

			if (ActiveBleManager == nullptr)
			{
				return;
			}

			String command = characteristic->getValue().c_str();
			command.trim();

			ActiveBleManager->receiveCommandFromCallback(command);
		}
	};

	BleServerCallbacks ServerCallbacks;
	BleCommandCallbacks CommandCallbacks;
}

bool BleManager::begin(const String &deviceName)
{
	Serial.println("Initializing BLE...");

	ActiveBleManager = this;

	if (!NimBLEDevice::init(deviceName.c_str()))
	{
		Serial.println("BLE initialization failed.");
		return false;
	}

	NimBLEDevice::setMTU(PreferredMtu);

	Serial.print("BLE preferred MTU: ");
	Serial.println(PreferredMtu);

	m_server = NimBLEDevice::createServer();

	if (m_server == nullptr)
	{
		Serial.println("BLE server creation failed.");
		return false;
	}

	m_server->setCallbacks(&ServerCallbacks);

	NimBLEService *service = m_server->createService(ServiceUuid);

	if (service == nullptr)
	{
		Serial.println("BLE service creation failed.");
		return false;
	}

	m_commandCharacteristic = service->createCharacteristic(
		CommandCharacteristicUuid,
		NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR,
		128
	);

	if (m_commandCharacteristic == nullptr)
	{
		Serial.println("BLE command characteristic creation failed.");
		return false;
	}

	m_commandCharacteristic->setCallbacks(&CommandCallbacks);

	m_responseCharacteristic = service->createCharacteristic(
		ResponseCharacteristicUuid,
		NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY,
		128
	);

	if (m_responseCharacteristic == nullptr)
	{
		Serial.println("BLE response characteristic creation failed.");
		return false;
	}

	m_responseCharacteristic->setValue("READY");

	m_server->start();

	NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();

	advertising->setName(deviceName.c_str());
	advertising->addServiceUUID(ServiceUuid);
	advertising->enableScanResponse(true);
	advertising->start();

	Serial.print("BLE initialized. Device name: ");
	Serial.println(deviceName);
	Serial.println("BLE service UUID:");
	Serial.println(ServiceUuid);
	Serial.println("BLE command characteristic UUID:");
	Serial.println(CommandCharacteristicUuid);
	Serial.println("BLE response characteristic UUID:");
	Serial.println(ResponseCharacteristicUuid);

	return true;
}

bool BleManager::getCommand(String &command)
{
	if (!m_hasPendingCommand)
	{
		return false;
	}

	command = m_pendingCommand;
	m_pendingCommand = "";
	m_hasPendingCommand = false;

	return true;
}

void BleManager::sendResponse(const String &response)
{
	Serial.print("BLE response: ");
	Serial.println(response);

	if (m_responseCharacteristic == nullptr)
	{
		return;
	}

	m_responseCharacteristic->setValue(response.c_str());
	m_responseCharacteristic->notify(
		reinterpret_cast<const uint8_t *>(response.c_str()),
		response.length()
	);
}

void BleManager::receiveCommandFromCallback(const String &command)
{
	if (command.length() == 0)
	{
		return;
	}

	m_pendingCommand = command;
	m_hasPendingCommand = true;

	Serial.print("BLE command received: ");
	Serial.println(command);
}