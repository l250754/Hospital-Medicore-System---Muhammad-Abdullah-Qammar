#include "Bill.h"

Bill::Bill() {
    billID = 0;
    patientID = 0;
    appointmentID = 0;
    amount = 0;
    status[0] = '\0';
    date[0] = '\0';
}
Bill::Bill(int billid, int patid, int appid, float amount, const char* status, const char* date) {
    billID = billid;
    patientID = patid;
    appointmentID = appid;
    this->amount = amount;

    int i = 0;
    while (status[i] != '\0' && i < 14) {
        this->status[i] = status[i];
        i++;
    }
    this->status[i] = '\0';

    i = 0;
    while (date[i] != '\0' && i < 10) {
        this->date[i] = date[i];
        i++;
    }
    this->date[i] = '\0';
}

Bill::~Bill() {}

Bill::Bill(const Bill& other) {
    billID = other.billID;
    patientID = other.patientID;
    appointmentID = other.appointmentID;
    amount = other.amount;

    int i = 0;
    while (other.status[i] != '\0') {
        status[i] = other.status[i];
        i++;
    }
    status[i] = '\0';

    i = 0;
    while (other.date[i] != '\0') {
        date[i] = other.date[i];
        i++;
    }
    date[i] = '\0';
}

int Bill::getbillid() const {
    return billID;
}

int Bill::getpatid() const {
    return patientID;
}

int Bill::getappid() const {
    return appointmentID;
}

float Bill::getamount() const {
    return amount;
}

const char* Bill::getstatus() const {
    return status;
}

const char* Bill::getdate() const {
    return date;
}

ostream& operator<<(ostream& out, const Bill& b) {
    out << b.billID << " | " << b.patientID << " | " << b.appointmentID << " | " << b.amount << " | " << b.status << " | " << b.date;
    return out;
}