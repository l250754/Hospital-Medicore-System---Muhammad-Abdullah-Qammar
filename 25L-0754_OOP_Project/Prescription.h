#pragma once
#include <iostream>
using namespace std;

class Prescription{
	int prescriptionID;
	int appointmentID;
	int patientID;
	int doctorID;
	char date[11];
	char medicines[500];
	char notes[300];
public:
	Prescription();
	Prescription(int, int, int, int, const char*, const char*, const char*);
	~Prescription();
	Prescription(const Prescription& other);
	int getprescid() const;
	int getdocid() const;
	int getpatid() const;
	int getappid() const;
	const char* getdate() const;
	const char* getmed() const;
	const char* getnotes() const;
	friend ostream& operator<<(ostream& out, const Prescription& p);
};

