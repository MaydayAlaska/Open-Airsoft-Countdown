#include "Users.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

namespace
{
	const char *UsersFilePath = "/users.json";
}

bool Users::begin()
{
	Serial.println("Initializing users...");

	clear();

	if (!load())
	{
		Serial.println("Failed to load users.json.");
		return false;
	}

	Serial.print("Users loaded: ");
	Serial.println(count());

	return true;
}

uint8_t Users::count() const
{
	uint8_t total = 0;

	for (uint8_t i = 0; i < MaxUsers; i++)
	{
		if (m_users[i].occupied)
		{
			total++;
		}
	}

	return total;
}

bool Users::getUserByPosition(uint8_t position, UserRecord &user) const
{
	uint8_t currentPosition = 0;

	for (uint8_t i = 0; i < MaxUsers; i++)
	{
		if (!m_users[i].occupied)
		{
			continue;
		}

		if (currentPosition == position)
		{
			user = m_users[i];
			return true;
		}

		currentPosition++;
	}

	return false;
}

bool Users::getUserById(uint16_t id, UserRecord &user) const
{
	const int index = findIndexById(id);

	if (index < 0)
	{
		return false;
	}

	user = m_users[index];
	return true;
}

bool Users::authenticate(const String &uid, const String &pin) const
{
	const int index = findIndexByUid(uid);

	if (index < 0)
	{
		return false;
	}

	return m_users[index].pin == pin;
}

bool Users::addUser(const String &name, const String &uid, const String &pin)
{
	if (!isValidName(name))
	{
		Serial.println("Invalid user name.");
		return false;
	}

	if (!isValidUid(uid))
	{
		Serial.println("Invalid UID.");
		return false;
	}

	if (!isValidPin(pin))
	{
		Serial.println("Invalid PIN. It must contain exactly 6 digits.");
		return false;
	}

	if (findIndexByUid(uid) >= 0)
	{
		Serial.println("UID already exists.");
		return false;
	}

	const int slot = findFreeSlot();

	if (slot < 0)
	{
		Serial.println("User database is full.");
		return false;
	}

	const uint16_t id = findFreeId();

	if (id == 0)
	{
		Serial.println("No free user ID available.");
		return false;
	}

	m_users[slot].occupied = true;
	m_users[slot].id = id;
	m_users[slot].name = name;
	m_users[slot].uid = uid;
	m_users[slot].pin = pin;

	if (!save())
	{
		m_users[slot] = UserRecord();
		return false;
	}

	return true;
}

bool Users::updateUser(uint16_t id, const String &name, const String &uid, const String &pin)
{
	if (!isValidName(name))
	{
		Serial.println("Invalid user name.");
		return false;
	}

	if (!isValidUid(uid))
	{
		Serial.println("Invalid UID.");
		return false;
	}

	if (!isValidPin(pin))
	{
		Serial.println("Invalid PIN. It must contain exactly 6 digits.");
		return false;
	}

	const int index = findIndexById(id);

	if (index < 0)
	{
		Serial.println("User not found.");
		return false;
	}

	const int uidIndex = findIndexByUid(uid);

	if (uidIndex >= 0 && uidIndex != index)
	{
		Serial.println("UID already exists.");
		return false;
	}

	const UserRecord previousUser = m_users[index];

	m_users[index].name = name;
	m_users[index].uid = uid;
	m_users[index].pin = pin;

	if (!save())
	{
		m_users[index] = previousUser;
		return false;
	}

	return true;
}

bool Users::removeUser(uint16_t id)
{
	const int index = findIndexById(id);

	if (index < 0)
	{
		return false;
	}

	const UserRecord previousUser = m_users[index];
	m_users[index] = UserRecord();

	if (!save())
	{
		m_users[index] = previousUser;
		return false;
	}

	return true;
}

bool Users::load()
{
	File file = LittleFS.open(UsersFilePath, "r");

	if (!file)
	{
		Serial.println("users.json not found. Creating empty users file...");
		return save();
	}

	JsonDocument document;
	DeserializationError error = deserializeJson(document, file);

	file.close();

	if (error)
	{
		Serial.println("users.json is invalid. Recreating empty users file...");
		clear();
		return save();
	}

	if (!document.is<JsonArray>())
	{
		Serial.println("users.json is not an array. Recreating empty users file...");
		clear();
		return save();
	}

	JsonArray usersArray = document.as<JsonArray>();

	for (JsonObject item : usersArray)
	{
		const int slot = findFreeSlot();

		if (slot < 0)
		{
			break;
		}

		const uint16_t id = item["id"] | 0;
		const char *name = item["name"] | "";
		const char *uid = item["uid"] | "";
		const char *pin = item["pin"] | "";

		if (id == 0 || !isValidName(name) || !isValidUid(uid) || !isValidPin(pin))
		{
			continue;
		}

		if (findIndexById(id) >= 0 || findIndexByUid(uid) >= 0)
		{
			continue;
		}

		m_users[slot].occupied = true;
		m_users[slot].id = id;
		m_users[slot].name = name;
		m_users[slot].uid = uid;
		m_users[slot].pin = pin;
	}

	return true;
}

bool Users::save() const
{
	File file = LittleFS.open(UsersFilePath, "w");

	if (!file)
	{
		return false;
	}

	JsonDocument document;
	JsonArray usersArray = document.to<JsonArray>();

	for (uint8_t i = 0; i < MaxUsers; i++)
	{
		if (!m_users[i].occupied)
		{
			continue;
		}

		JsonObject item = usersArray.add<JsonObject>();

		item["id"] = m_users[i].id;
		item["name"] = m_users[i].name;
		item["uid"] = m_users[i].uid;
		item["pin"] = m_users[i].pin;
	}

	const size_t bytesWritten = serializeJsonPretty(document, file);

	file.close();

	return bytesWritten > 0;
}

void Users::clear()
{
	for (uint8_t i = 0; i < MaxUsers; i++)
	{
		m_users[i] = UserRecord();
	}
}

bool Users::isValidPin(const String &pin) const
{
	if (pin.length() != PinLength)
	{
		return false;
	}

	for (uint8_t i = 0; i < PinLength; i++)
	{
		if (pin[i] < '0' || pin[i] > '9')
		{
			return false;
		}
	}

	return true;
}

bool Users::isValidName(const String &name) const
{
	return name.length() > 0 && name.length() <= MaxNameLength;
}

bool Users::isValidUid(const String &uid) const
{
	if (uid.length() == 0 || uid.length() > MaxUidLength)
	{
		return false;
	}

	for (uint8_t i = 0; i < uid.length(); i++)
	{
		const char character = uid[i];
		const bool isNumber = character >= '0' && character <= '9';
		const bool isUpperHex = character >= 'A' && character <= 'F';

		if (!isNumber && !isUpperHex)
		{
			return false;
		}
	}

	return true;
}

int Users::findFreeSlot() const
{
	for (uint8_t i = 0; i < MaxUsers; i++)
	{
		if (!m_users[i].occupied)
		{
			return i;
		}
	}

	return -1;
}

int Users::findIndexById(uint16_t id) const
{
	for (uint8_t i = 0; i < MaxUsers; i++)
	{
		if (m_users[i].occupied && m_users[i].id == id)
		{
			return i;
		}
	}

	return -1;
}

int Users::findIndexByUid(const String &uid) const
{
	for (uint8_t i = 0; i < MaxUsers; i++)
	{
		if (m_users[i].occupied && m_users[i].uid == uid)
		{
			return i;
		}
	}

	return -1;
}

uint16_t Users::findFreeId() const
{
	for (uint16_t id = 1; id < 65535; id++)
	{
		if (findIndexById(id) < 0)
		{
			return id;
		}
	}

	return 0;
}
