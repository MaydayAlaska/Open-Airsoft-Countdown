#pragma once

#include <Arduino.h>

struct UserRecord
{
	bool occupied;
	uint16_t id;
	String name;
	String uid;
	String pin;
};

class Users
{
public:
	bool begin();

	uint8_t count() const;

	bool authenticate(const String &uid, const String &pin) const;

	bool addUser(const String &name, const String &uid, const String &pin);
	bool removeUser(uint16_t id);

private:
	static constexpr uint8_t MaxUsers = 100;
	static constexpr uint8_t PinLength = 6;

	bool load();
	bool save() const;

	void clear();

	bool isValidPin(const String &pin) const;

	int findFreeSlot() const;
	int findIndexById(uint16_t id) const;
	int findIndexByUid(const String &uid) const;

	uint16_t findFreeId() const;

	UserRecord m_users[MaxUsers];
};