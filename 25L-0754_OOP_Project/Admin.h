#pragma once
#include "Person.h"

class Admin final :public Person {
public:
	Admin();
	Admin(const char*, int, const char*);
	~Admin();
	Admin(const Admin& other);
	void displayMenu() override;
	void display() override;
};