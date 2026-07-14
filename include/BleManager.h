#pragma once

#include <Arduino.h>

class NimBLEServer;
class NimBLECharacteristic;

class BleManager
{
public:
	bool begin(const String &deviceName);

	bool getCommand(String &command);
	void sendResponse(const String &response);

	void receiveCommandFromCallback(const String &command);

private:
	static constexpr const char *ServiceUuid = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
	static constexpr const char *CommandCharacteristicUuid = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
	static constexpr const char *ResponseCharacteristicUuid = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

	NimBLEServer *m_server = nullptr;
	NimBLECharacteristic *m_commandCharacteristic = nullptr;
	NimBLECharacteristic *m_responseCharacteristic = nullptr;

	String m_pendingCommand;
	bool m_hasPendingCommand = false;
};