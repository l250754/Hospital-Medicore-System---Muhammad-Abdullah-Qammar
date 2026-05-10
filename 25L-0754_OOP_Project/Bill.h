#pragma once
#include <iostream>
using namespace std;
class Bill {
	int billID;
	int patientID;
	int appointmentID;
	float amount;
	char status[15];
	char date[11];
public:
	Bill();
	Bill(int billid, int patid, int appid, float amount, const char* status, const char* date);
	~Bill();
	Bill(const Bill& other);
	int getbillid() const;
	int getpatid() const;
	const char* getdate() const;
	float getamount() const;
	const char* getstatus() const;
	int getappid() const;
	friend ostream& operator<<(ostream& out, const Bill& b);
};