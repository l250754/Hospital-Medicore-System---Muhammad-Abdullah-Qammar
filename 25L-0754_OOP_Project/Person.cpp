#include"Person.h"
#include <iostream>
using namespace std;

Person::Person() {
	name = new char[1]; name[0] = '\0';
	id = 0;
	password = new char[1]; password[0] = '\0';
}
Person::Person(const char* n, int id, const char* pass) {
	int len1 = 0;
	while (n[len1] != '\0') len1++;
	name = new char[len1 + 1];

	this->id = id;

	int len2 = 0;
	while (pass[len2] != '\0') len2++;
	password = new char[len2 + 1];

	for (int i = 0; i <= len1; i++) name[i] = n[i];	
	for (int i = 0; i <= len2; i++) password[i] = pass[i];
}
Person::Person(const Person& other) {
	int len1 = 0;
	while (other.name[len1] != '\0') len1++;
	name = new char[len1 + 1];
	
	id = other.id;

	int len2 = 0;
	while (other.password[len2] != '\0') len2++;
	password = new char[len2 + 1];

	for (int i = 0; i <= len1; i++) name[i] = other.name[i];	
	for (int i = 0; i <= len2; i++) password[i] = other.password[i];
}
Person& Person::operator=(const Person& other) {
	if (this == &other) return *this;

	char* newName;
	char* newPassword;

	int len1 = 0;
	while (other.name[len1] != '\0') len1++;
	newName = new char[len1 + 1];
	for (int i = 0; i <= len1; i++) newName[i] = other.name[i];

	int len2 = 0;
	while (other.password[len2] != '\0') len2++;
	newPassword = new char[len2 + 1];
	for (int i = 0; i <= len2; i++) newPassword[i] = other.password[i];

	delete[] name;
	delete[] password;

	name = newName;
	password = newPassword;
	id = other.id;

	return *this;
}
const char* Person::getName() const {
	return name; 
}
int Person::getId() const {
	return id;
}
const char* Person::getPassword() const {
	return password;
}
Person::~Person() {
	delete[] name;
	delete[] password;
}