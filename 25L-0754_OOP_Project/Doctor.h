#pragma once
#include"Person.h"
#include<iostream>
using namespace std;

class Doctor :public Person {
	char* specialization;
	char* contact;
	float fee;
public:
	Doctor();
	Doctor(const char*, int, const char*, const char*, const char*,float);
	~Doctor();
	Doctor(const Doctor& other);
	Doctor& operator=(const Doctor& other);
	const char* Getspecialization() const;
	const char* Getcontact() const ;
	float Getfee() const;
	bool operator==(const Doctor& other);
	friend ostream& operator<<(ostream& out, const Doctor& d);
	void displayMenu() override;
	void display() override;
};
