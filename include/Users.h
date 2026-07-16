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
	bool getUserByPosition(uint8_t position, UserRecord &user) const;
	bool getUserById(uint16_t id, UserRecord &user) const;
	bool getUserByUid(const String &uid, UserRecord &user) const;

	bool authenticate(const String &uid, const String &pin) const;

	bool addUser(const String &name, const String &uid, const String &pin);
	bool updateUser(uint16_t id, const String &name, const String &uid, const String &pin);
	bool removeUser(uint16_t id);

private:
	static constexpr uint8_t MaxUsers = 100;
	static constexpr uint8_t PinLength = 6;
	static constexpr uint8_t MaxNameLength = 32;
	static constexpr uint8_t MaxUidLength = 32;

	bool load();
	bool save() const;

	void clear();

	bool isValidPin(const String &pin) const;
	bool isValidName(const String &name) const;
	bool isValidUid(const String &uid) const;

	int findFreeSlot() const;
	int findIndexById(uint16_t id) const;
	int findIndexByUid(const String &uid) const;

	uint16_t findFreeId() const;

	UserRecord m_users[MaxUsers];
};
