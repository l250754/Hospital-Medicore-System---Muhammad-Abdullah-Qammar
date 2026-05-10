#include "Patient.h"

Patient::Patient() : Person() {
	contact = new char[1]; contact[0] = '\0';
	age = 0;
	gender = 'M';
	balance = 0;
}
Patient::Patient(const char* name, int id, const char* password, const char* contact, int age, char gender, float balance) : Person(name, id, password) {
	int len1 = 0;
	while (contact[len1] != '\0') len1++;
	this->contact = new char[len1 + 1];	
	for (int i = 0; i <= len1; i++) this->contact[i] = contact[i];
	this->age = age;
	this->gender = gender;
	this->balance = balance;
}
Patient::~Patient() {
	delete[] contact;
}
Patient::Patient(const Patient& other) : Person(other) {
	int len1 = 0;
	while (other.contact[len1] != '\0') len1++;
	this->contact = new char[len1 + 1];
	for (int i = 0; i <= len1; i++) this->contact[i] = other.contact[i];
	this->age = other.age;
	this->gender = other.gender;
	this->balance = other.balance;
}
Patient& Patient::operator=(const Patient& other) {
	if (this == &other) return *this;

	Person::operator=(other);

	int len1 = 0;
	while (other.contact[len1] != '\0') len1++;
	char* newContact = new char[len1 + 1];
	for (int i = 0; i <= len1; i++) newContact[i] = other.contact[i];

	delete[] contact;
	contact = newContact;
	age = other.age;
	gender = other.gender;
	balance = other.balance;

	return *this;
}
const char* Patient::getContact() const {
	return contact;
}
int Patient::getAge() const {
	return age;
}
char Patient::getGender() const {
	return gender;
}
float Patient::getBalance() const {
	return balance;
}
bool Patient::operator==(const Patient& other) {
	int a = getId();
	int b = other.getId();
	if (a == b)	{
		return true;
	}
	return false;
}
Patient& Patient::operator+=(float amount) {
	balance += amount;
	return *this;
}
Patient& Patient::operator-=(float amount) {
	balance -= amount;
	return *this;
}
ostream& operator<<(ostream& out, const Patient& p) {
	out << "ID: " << p.id
		<< " Name: " << p.name
		<< " Age: " << p.age
		<< " Gender: " << p.gender
		<< " Contact: " << p.contact
		<< " Balance: " << p.balance;
	return out;
}

void Patient::displayMenu() {
	cout << "1. Book Appointment" << endl;
	cout << "2. Cancel Appointment" << endl;
	cout << "3. View My Appointments" << endl;
	cout << "4. View My Medical Records" << endl;
	cout << "5. View My Bills" << endl;
	cout << "6. Pay Bill" << endl;
	cout << "7. Top Up Balance" << endl;
	cout << "8. Logout" << endl;
}

void Patient::display() {
	cout << *this << endl;
}