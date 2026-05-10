#pragma once
#include <iostream>
using namespace std;

class Appointment {
	int appointmentID;
	int patientID;
	int doctorID;
	char date[11];
	char timeslot[6];
	char status[15];
public:
	Appointment();
	Appointment(int appid, int patid, int docid, const char* date, const char* time, const char* status);
	~Appointment();
	Appointment(const Appointment& other);
	int getappid() const;
	int getpatid() const;
	int getdocid() const;
	const char* getdate() const;
	const char* gettime() const;
	const char* getstatus() const;
	bool operator==(const Appointment& other) const;
	friend ostream& operator<<(ostream& out, const Appointment& a);
};