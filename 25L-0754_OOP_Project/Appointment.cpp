#include "Appointment.h"

Appointment::Appointment() {
	appointmentID = 0;
	patientID = 0;
	doctorID = 0;
	date[0] = '\0';
	timeslot[0] = '\0';
	status[0] = '\0';
}
Appointment::Appointment(int appid, int patid, int docid, const char* date, const char* time, const char* status) {
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
	while (time[i] != '\0' && i < 5) {
		this->timeslot[i] = time[i];
		i++;
	}
	this->timeslot[i] = '\0';

	i = 0;
	while (status[i] != '\0' && i < 14) {
		this->status[i] = status[i];
		i++;
	}
	this->status[i] = '\0';
}

Appointment::~Appointment() {}

Appointment::Appointment(const Appointment& other) {
	appointmentID = other.appointmentID;
	patientID = other.patientID;
	doctorID = other.doctorID;

	int i = 0;
	while (other.date[i] != '\0') {
		this->date[i] = other.date[i];
		i++;
	}
	this->date[i] = '\0';

	i = 0;
	while (other.timeslot[i] != '\0') {
		this->timeslot[i] = other.timeslot[i];
		i++;
	}
	this->timeslot[i] = '\0';

	i = 0;
	while (other.status[i] != '\0') {
		this->status[i] = other.status[i];
		i++;
	}
	this->status[i] = '\0';
}

int Appointment::getappid() const {
	return appointmentID;
}

int Appointment::getpatid() const {
	return patientID;
}

int Appointment::getdocid() const {
	return doctorID;
}

const char* Appointment::getdate() const {
	return date;
}

const char* Appointment::gettime() const {
	return timeslot;
}

const char* Appointment::getstatus() const {
	return status;
}

bool Appointment::operator==(const Appointment& other) const {
	if (doctorID != other.doctorID) return false;

	int i = 0;
	while (date[i] != '\0' && other.date[i] != '\0') {
		if (date[i] != other.date[i]) return false;
		i++;
	}
	if (date[i] != other.date[i]) return false;

	i = 0;
	while (timeslot[i] != '\0' && other.timeslot[i] != '\0') {
		if (timeslot[i] != other.timeslot[i]) return false;
		i++;
	}
	if (timeslot[i] != other.timeslot[i]) return false;

	bool thisCancelled = true;
	i = 0;
	while ("cancelled"[i] != '\0') {
		if (status[i] != "cancelled"[i]) { thisCancelled = false; break; }
		i++;
	}
	bool otherCancelled = true;
	i = 0;
	while ("cancelled"[i] != '\0') {
		if (other.status[i] != "cancelled"[i]) { otherCancelled = false; break; }
		i++;
	}
	if (thisCancelled || otherCancelled) return false;

	return true;
}

ostream& operator<<(ostream& out, const Appointment& a) {
	out << a.appointmentID << " | " << a.patientID << " | " << a.doctorID << " | " << a.date << " | " << a.timeslot << " | " << a.status;
	return out;
}