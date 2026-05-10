#include "Doctor.h"

Doctor::Doctor() : Person() {
	specialization = new char[1]; specialization[0] = '\0';
	contact = new char[1]; contact[0] = '\0';
	fee = 0;
}
Doctor::Doctor(const char* name, int id, const char* password, const char* specialization, const char* contact, float fee) :Person(name, id, password) {
	int len1 = 0;
	while (specialization[len1] != '\0') len1++;
	this->specialization = new char[len1 + 1];
	int len2 = 0;
	while (contact[len2] != '\0') len2++;
	this->contact = new char[len2 + 1];
	
	for (int i = 0; i <= len1; i++) this->specialization[i] = specialization[i];
	for (int i = 0; i <= len2; i++) this->contact[i] = contact[i];
	this->fee = fee;
}
Doctor::~Doctor() {
	delete[] specialization;
	delete[] contact;
}
Doctor::Doctor(const Doctor& other) : Person(other) {
	int len1 = 0;
	while (other.specialization[len1] != '\0') len1++;
	this->specialization = new char[len1 + 1];
	int len2 = 0;
	while (other.contact[len2] != '\0') len2++;
	this->contact = new char[len2 + 1];

	for (int i = 0; i <= len1; i++) this->specialization[i] = other.specialization[i];
	for (int i = 0; i <= len2; i++) this->contact[i] = other.contact[i];
	this->fee = other.fee;
}
Doctor& Doctor::operator=(const Doctor& other) {
	if (this == &other) return *this;

	Person::operator=(other);

	int len1 = 0;
	while (other.specialization[len1] != '\0') len1++;
	char* newSpecialization = new char[len1 + 1];
	for (int i = 0; i <= len1; i++) newSpecialization[i] = other.specialization[i];

	int len2 = 0;
	while (other.contact[len2] != '\0') len2++;
	char* newContact = new char[len2 + 1];
	for (int i = 0; i <= len2; i++) newContact[i] = other.contact[i];

	delete[] specialization;
	delete[] contact;

	specialization = newSpecialization;
	contact = newContact;
	fee = other.fee;

	return *this;
}
const char* Doctor::Getspecialization() const {
	return specialization;
}
const char* Doctor::Getcontact() const {
	return contact;
}
float Doctor::Getfee() const {
	return fee;
}
bool Doctor::operator==(const Doctor& other) {
	int a = getId();
	int b = other.getId();
	if (a == b) {
		return true;
	}
	return false;
}
ostream& operator<<(ostream& out, const Doctor& d) {
	out << "ID: " << d.getId()
		<< " Name: " << d.getName()
		<< " Specialization: " << d.specialization  
		<< " Fee: " << d.fee;                       
	return out;
}
void Doctor::displayMenu() {
	cout << "1. View Today's Appointments" << endl;
	cout << "2. Mark Appointment Complete" << endl;
	cout << "3. Mark Appointment No-Show" << endl;
	cout << "4. Write Prescription" << endl;
	cout << "5. View Patient Medical History" << endl;
	cout << "6. Logout" << endl;
}
void Doctor::display() {
	cout << *this << endl;
}