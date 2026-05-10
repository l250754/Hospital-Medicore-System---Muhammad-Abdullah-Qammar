#pragma once
#include"Person.h"
#include<iostream>
using namespace std;

class Patient : public Person {
	char* contact;
	int age;
	char gender;
	float balance;
public:
	Patient();
	Patient(const char*, int, const char*, const char*, int, char, float);
	~Patient();
	Patient(const Patient& other);
	Patient& operator=(const Patient& other);
	const char* getContact() const;
	int getAge() const;
	char getGender() const; 
	float getBalance() const;
	bool operator==(const Patient& other);
	Patient& operator+=(float amount);
	Patient& operator-=(float amount);
	friend ostream& operator<<(ostream& out, const Patient& d);
	void displayMenu() override;
	void display() override;
};