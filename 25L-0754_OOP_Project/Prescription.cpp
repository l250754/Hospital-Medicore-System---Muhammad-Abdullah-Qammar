#include "Prescription.h"

Prescription::Prescription() {
	prescriptionID = 0;
	appointmentID = 0;
	patientID = 0;
	doctorID = 0;
	date[0] = '\0';
	medicines[0] = '\0';
	notes[0] = '\0';
}
Prescription::Prescription(int prescid, int appid, int patid, int docid, const char* date, const char* med, const char* notes) {
	prescriptionID = prescid;
	appointmentID = appid;
	patientID = patid;
	doctorID = docid;
	int i = 0;
	while (date[i] != '\0' && i < 10) {
		this->date[i] = date[i];
		i++;
	}
	this->date[i] = '\0';
	i = 0;
	while (med[i] != '\0' && i < 499) {
		this->medicines[i] = med[i];
		i++;
	}
	this->medicines[i] = '\0';

	i = 0;
	while (notes[i] != '\0' && i < 299) {
		this->notes[i] = notes[i];
		i++;
	}
	this->notes[i] = '\0';
}
Prescription::~Prescription() {}

Prescription::Prescription(const Prescription& other) {
	prescriptionID = other.prescriptionID;
	appointmentID = other.appointmentID;
	patientID = other.patientID;
	doctorID = other.doctorID;
	int i = 0;
	while (other.date[i] != '\0' && i < 10) {
		this->date[i] = other.date[i];
		i++;
	}
	this->date[i] = '\0';

	i = 0;
	while (other.medicines[i] != '\0' && i < 499) {
		this->medicines[i] = other.medicines[i];
		i++;
	}
	this->medicines[i] = '\0';

	i = 0;
	while (other.notes[i] != '\0' && i < 299) {
		this->notes[i] = other.notes[i];
		i++;
	}
	this->notes[i] = '\0';
}
int Prescription::getprescid() const {
	return prescriptionID;
}
int Prescription::getdocid() const {
	return doctorID;
}
int Prescription::getpatid() const {
	return patientID;
}
int Prescription::getappid() const {
	return appointmentID;
}
const char* Prescription::getdate() const {
	return date;
}
const char* Prescription::getmed() const {
	return medicines;
}
const char* Prescription::getnotes() const {
	return notes;
}
ostream& operator<<(ostream& out, const Prescription& p) {
	out << p.prescriptionID << " | " << p.appointmentID << " | "
		<< p.patientID << " | " << p.doctorID << " | "
		<< p.date << " | " << p.medicines << " | " << p.notes;
	return out;
}