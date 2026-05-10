#define _CRT_SECURE_NO_WARNINGS
#include "FileHandler.h"
#include "Exceptions.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdio>

using namespace std;

void FileHandler::initializeFiles() {
    const char* filenames[] = { "patients.txt", "doctors.txt", "admin.txt", "appointments.txt", "bills.txt", "prescriptions.txt", "discharged.txt", "security_log.txt", "account_locks.txt" };
    const char* headers[] = {
        "patient_id,name,age,gender,contact,password,balance",
        "doctor_id,name,specialization,contact,password,fee",
        "admin_id,name,password",
        "appointment_id,patient_id,doctor_id,date,time_slot,status",
        "bill_id,patient_id,appointment_id,amount,status,date",
        "prescription_id,appointment_id,patient_id,doctor_id,date,medicines,notes",
        "archive_type,data",
        "timestamp,role,entered_id,result",
        "role,id,failed_attempts,is_locked"
    };

    for (int i = 0; i < 9; i++) {
        ifstream check(filenames[i]);
        if (!check.is_open()) {
            ofstream create(filenames[i]);
            create << headers[i] << endl;
            if (i == 2) {
                create << "1,MainAdmin,admin123" << endl;
            }
            create.close();
        }
        check.close();
    }
}

void FileHandler::loadPatients(Storage<Patient>& pat) {
   pat.clear();
   ifstream file("patients.txt");
   if (!file.is_open()) return;
   char line[500];
   file.getline(line, 500); // Skip Header
   while (file.getline(line, 500)) {
       if (line[0] == '\0') continue;
       char id[20], name[100], age[10], gen[20], contact[20], pass[50], bal[20];
               
       extractField(line, 0, id);     
       extractField(line, 1, name);   
       extractField(line, 2, age);     
       extractField(line, 3, gen);     
       extractField(line, 4, contact); 
       extractField(line, 5, pass);    
       extractField(line, 6, bal);   

       if (pat.findById(toInt(id), getPatId) != nullptr) continue;
       char gender = 'M';
       if (gen[0] != '\0') gender = gen[0];
       Patient p(name, toInt(id), pass, contact, toInt(age), gender, toFloat(bal));
       pat.add(p);
   }
   file.close();

   ofstream out("patients.txt", ios::trunc);
   out << "patient_id,name,age,gender,contact,password,balance" << endl;
   Patient* all = pat.getAll();
   for (int i = 0; i < pat.size(); i++) {
       out << all[i].getId() << ","
           << all[i].getName() << ","
           << all[i].getAge() << ","
           << all[i].getGender() << ","
           << all[i].getContact() << ","
           << all[i].getPassword() << ","
           << all[i].getBalance() << endl;
   }
   out.close();
}

void FileHandler::loadDoctors(Storage<Doctor>& storage) {
    storage.clear();
    ifstream file("doctors.txt");
    if (!file.is_open()) return;
    char line[500];
    file.getline(line, 500);
    while (file.getline(line, 500)) {
        if (line[0] == '\0') continue;
        char id[20], name[100], spec[100], contact[20], pass[50], fee[20];
        extractField(line, 0, id); extractField(line, 1, name);
        extractField(line, 2, spec); extractField(line, 3, contact);
        extractField(line, 4, pass); extractField(line, 5, fee);
        if (storage.findById(toInt(id), getDocId) != nullptr) continue;
        Doctor d(name, toInt(id), pass, spec, contact, toFloat(fee));
        storage.add(d);
    }
    file.close();

    ofstream out("doctors.txt", ios::trunc);
    out << "doctor_id,name,specialization,contact,password,fee" << endl;
    Doctor* all = storage.getAll();
    for (int i = 0; i < storage.size(); i++) {
        out << all[i].getId() << ","
            << all[i].getName() << ","
            << all[i].Getspecialization() << ","
            << all[i].Getcontact() << ","
            << all[i].getPassword() << ","
            << all[i].Getfee() << endl;
    }
    out.close();
}

void FileHandler::loadAdmin(Admin& admin) {
    ifstream file("admin.txt");
    if (!file.is_open()) return;
    char line[500];
    file.getline(line, 500);
    bool loaded = false;
    if (file.getline(line, 500)) {
        char id[20], name[100], pass[50];
        extractField(line, 0, id); extractField(line, 1, name); extractField(line, 2, pass);
        admin = Admin(name, toInt(id), pass);
        loaded = true;
    }
    file.close();

    if (loaded) {
        ofstream out("admin.txt", ios::trunc);
        out << "admin_id,name,password" << endl;
        out << admin.getId() << "," << admin.getName() << "," << admin.getPassword() << endl;
        out.close();
    }
}

void FileHandler::loadAppointments(Storage<Appointment>& storage) {
    storage.clear();
    ifstream file("appointments.txt");
    if (!file.is_open()) return;
    char line[500];
    file.getline(line, 500);
    while (file.getline(line, 500)) {
        if (line[0] == '\0') continue;
        char appID[20], patID[20], docID[20], date[15], time[15], status[20];
        extractField(line, 0, appID); extractField(line, 1, patID);
        extractField(line, 2, docID); extractField(line, 3, date);
        extractField(line, 4, time); extractField(line, 5, status);
        Appointment a(toInt(appID), toInt(patID), toInt(docID), date, time, status);
        storage.add(a);
    }
    file.close();
}

void FileHandler::loadBills(Storage<Bill>& storage) {
    storage.clear();
    ifstream file("bills.txt");
    if (!file.is_open()) return;
    char line[500];
    file.getline(line, 500);
    while (file.getline(line, 500)) {
        if (line[0] == '\0') continue;
        char bID[20], pID[20], aID[20], amt[20], stat[20], date[15];
        extractField(line, 0, bID); extractField(line, 1, pID);
        extractField(line, 2, aID); extractField(line, 3, amt);
        extractField(line, 4, stat); extractField(line, 5, date);
        Bill b(toInt(bID), toInt(pID), toInt(aID), toFloat(amt), stat, date);
        storage.add(b);
    }
    file.close();
}

void FileHandler::loadPrescriptions(Storage<Prescription>& storage) {
    storage.clear();
    ifstream file("prescriptions.txt");
    if (!file.is_open()) return;
    char line[500];
    file.getline(line, 500);
    while (file.getline(line, 500)) {
        if (line[0] == '\0') continue;
        char prID[20], aID[20], pID[20], dID[20], date[15], meds[500], notes[300];
        extractField(line, 0, prID); extractField(line, 1, aID);
        extractField(line, 2, pID); extractField(line, 3, dID);
        extractField(line, 4, date); extractField(line, 5, meds); extractField(line, 6, notes);
        Prescription pr(toInt(prID), toInt(aID), toInt(pID), toInt(dID), date, meds, notes);
        storage.add(pr);
    }
    file.close();
}

void FileHandler::appendPatient(const Patient& p) {
    ofstream file("patients.txt", ios::app);
    if (file.is_open()) {
       
        file << p.getId() << ","
            << p.getName() << ","
            << p.getAge() << ","
            << p.getGender() << ","
            << p.getContact() << ","
            << p.getPassword() << ","
            << p.getBalance() << endl;
        file.close();
    }
}
void FileHandler::appendDoctor(const Doctor& d) {
    ofstream file("doctors.txt", ios::app);
    if (file.is_open()) {
        file << d.getId() << "," << d.getName() << "," << d.Getspecialization() << ","
            << d.Getcontact() << "," << d.getPassword() << "," << d.Getfee() << endl;
        file.close();
    }
}

void FileHandler::appendAppointment(const Appointment& a) {
    ofstream file("appointments.txt", ios::app);
    if (file.is_open()) {
        file << a.getappid() << "," << a.getpatid() << "," << a.getdocid() << ","
            << a.getdate() << "," << a.gettime() << "," << a.getstatus() << endl;
        file.close();
    }
}

void FileHandler::appendBill(const Bill& b) {
    ofstream file("bills.txt", ios::app);
    if (file.is_open()) {
        file << b.getbillid() << "," << b.getpatid() << "," << b.getappid() << ","
            << b.getamount() << "," << b.getstatus() << "," << b.getdate() << endl;
        file.close();
    }
}

void FileHandler::appendPrescription(const Prescription& p) {
    ofstream file("prescriptions.txt", ios::app);
    if (file.is_open()) {
        file << p.getprescid() << "," << p.getappid() << "," << p.getpatid() << ","
            << p.getdocid() << "," << p.getdate() << "," << p.getmed() << ","
            << p.getnotes() << endl;
        file.close();
    }
}

void FileHandler::updatePatient(const Patient& p) {
    Storage<Patient> temp;
    loadPatients(temp);
    Patient* data = temp.getAll();
    ofstream file("patients.txt", ios::trunc);
    file << "patient_id,name,age,gender,contact,password,balance" << endl;
    for (int i = 0; i < temp.size(); i++) {
        if (data[i].getId() == p.getId()) data[i] = p;
        file << data[i].getId() << "," << data[i].getName() << "," << data[i].getAge() << ","
            << data[i].getGender() << "," << data[i].getContact() << "," << data[i].getPassword() << ","
            << data[i].getBalance() << endl;
    }
}

void FileHandler::updateAppointment(const Appointment& a) {
    Storage<Appointment> temp;
    loadAppointments(temp);
    Appointment* data = temp.getAll();
    ofstream file("appointments.txt", ios::trunc);
    file << "appointment_id,patient_id,doctor_id,date,time_slot,status" << endl;
    for (int i = 0; i < temp.size(); i++) {
        if (data[i].getappid() == a.getappid()) data[i] = a;
        file << data[i].getappid() << "," << data[i].getpatid() << "," << data[i].getdocid() << ","
            << data[i].getdate() << "," << data[i].gettime() << "," << data[i].getstatus() << endl;
    }
}

void FileHandler::updateBill(const Bill& b) {
    Storage<Bill> temp;
    loadBills(temp);
    Bill* data = temp.getAll();
    ofstream file("bills.txt", ios::trunc);
    file << "bill_id,patient_id,appointment_id,amount,status,date" << endl;
    for (int i = 0; i < temp.size(); i++) {
        if (data[i].getbillid() == b.getbillid()) data[i] = b;
        file << data[i].getbillid() << "," << data[i].getpatid() << "," << data[i].getappid() << ","
            << data[i].getamount() << "," << data[i].getstatus() << "," << data[i].getdate() << endl;
    }
}

void FileHandler::deleteDoctor(int id) {
    Storage<Doctor> temp;
    loadDoctors(temp);
    Doctor* data = temp.getAll();
    ofstream file("doctors.txt", ios::trunc);
    file << "doctor_id,name,specialization,contact,password,fee" << endl;
    for (int i = 0; i < temp.size(); i++) {
        if (data[i].getId() != id) {
            file << data[i].getId() << "," << data[i].getName() << "," << data[i].Getspecialization() << ","
                << data[i].Getcontact() << "," << data[i].getPassword() << "," << data[i].Getfee() << endl;
        }
    }
}

void FileHandler::deletePatient(int id) {
    Storage<Patient> temp;
    loadPatients(temp);
    Patient* data = temp.getAll();
    ofstream file("patients.txt", ios::trunc);
    file << "patient_id,name,age,gender,contact,password,balance" << endl;
    for (int i = 0; i < temp.size(); i++) {
        if (data[i].getId() != id) {
            file << data[i].getId() << "," << data[i].getName() << "," << data[i].getAge() << ","
                << data[i].getGender() << "," << data[i].getContact() << "," << data[i].getPassword() << ","
                << data[i].getBalance() << endl;
        }
    }
}

void FileHandler::logSecurity(const char* role, int id, const char* result) {
    ofstream file("security_log.txt", ios::app);
    if (file.is_open()) {
        time_t now = time(0);
        tm localNow;
        localtime_s(&localNow, &now);
        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%d-%m-%Y %H:%M:%S", &localNow);
        file << timestamp << "," << role << "," << id << "," << result << endl;
        file.close();
    }
}

int FileHandler::getMaxId(const char* filename) {
    ifstream file(filename);
    if (!file.is_open()) return 0;
    char line[500];
    int maxId = 0;
    file.getline(line, 500);
    while (file.getline(line, 500)) {
        char idStr[20] = { 0 };
        extractField(line, 0, idStr);
        int currentId = toInt(idStr);
        if (currentId > maxId) maxId = currentId;
    }
    file.close();
    return maxId;
}

bool FileHandler::idExists(const char* filename, int id) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    char line[500];
    file.getline(line, 500);
    while (file.getline(line, 500)) {
        if (line[0] == '\0') continue;
        char idStr[20] = { 0 };
        extractField(line, 0, idStr);
        if (toInt(idStr) == id) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

int FileHandler::getNextUniqueId(const char* filename) {
    int candidate = getMaxId(filename) + 1;
    while (idExists(filename, candidate)) candidate++;
    return candidate;
}

bool FileHandler::isAccountLocked(const char* role, int id) {
    ifstream file("account_locks.txt");
    if (!file.is_open()) return false;
    char line[500];
    file.getline(line, 500);
    while (file.getline(line, 500)) {
        if (line[0] == '\0') continue;
        char fileRole[20], fileId[20], fileFailed[20], fileLocked[20];
        extractField(line, 0, fileRole);
        extractField(line, 1, fileId);
        extractField(line, 2, fileFailed);
        extractField(line, 3, fileLocked);
        if (stringCompare(fileRole, role) && toInt(fileId) == id) {
            file.close();
            return toInt(fileLocked) == 1;
        }
    }
    file.close();
    return false;
}

void FileHandler::recordFailedLogin(const char* role, int id) {
    ifstream in("account_locks.txt");
    ofstream out("account_locks_temp.txt");
    char line[500];
    bool found = false;
    if (in.is_open()) in.getline(line, 500);
    out << "role,id,failed_attempts,is_locked" << endl;
    while (in.is_open() && in.getline(line, 500)) {
        if (line[0] == '\0') continue;
        char fileRole[20], fileId[20], fileFailed[20], fileLocked[20];
        extractField(line, 0, fileRole);
        extractField(line, 1, fileId);
        extractField(line, 2, fileFailed);
        extractField(line, 3, fileLocked);
        if (stringCompare(fileRole, role) && toInt(fileId) == id) {
            found = true;
            int failed = toInt(fileFailed) + 1;
            int locked = (failed >= 3) ? 1 : toInt(fileLocked);
            out << role << "," << id << "," << failed << "," << locked << endl;
        }
        else {
            out << fileRole << "," << toInt(fileId) << "," << toInt(fileFailed) << "," << toInt(fileLocked) << endl;
        }
    }
    if (!found) {
        int failed = 1;
        int locked = 0;
        out << role << "," << id << "," << failed << "," << locked << endl;
    }
    if (in.is_open()) in.close();
    out.close();
    remove("account_locks.txt");
    rename("account_locks_temp.txt", "account_locks.txt");
}

void FileHandler::resetFailedLogin(const char* role, int id) {
    ifstream in("account_locks.txt");
    ofstream out("account_locks_temp.txt");
    char line[500];
    bool found = false;
    if (in.is_open()) in.getline(line, 500);
    out << "role,id,failed_attempts,is_locked" << endl;
    while (in.is_open() && in.getline(line, 500)) {
        if (line[0] == '\0') continue;
        char fileRole[20], fileId[20], fileFailed[20], fileLocked[20];
        extractField(line, 0, fileRole);
        extractField(line, 1, fileId);
        extractField(line, 2, fileFailed);
        extractField(line, 3, fileLocked);
        if (stringCompare(fileRole, role) && toInt(fileId) == id) {
            found = true;
            out << role << "," << id << ",0,0" << endl;
        }
        else {
            out << fileRole << "," << toInt(fileId) << "," << toInt(fileFailed) << "," << toInt(fileLocked) << endl;
        }
    }
    if (!found) {
        out << role << "," << id << ",0,0" << endl;
    }
    if (in.is_open()) in.close();
    out.close();
    remove("account_locks.txt");
    rename("account_locks_temp.txt", "account_locks.txt");
}

bool FileHandler::unlockAccount(const char* role, int id) {
    ifstream in("account_locks.txt");
    ofstream out("account_locks_temp.txt");
    char line[500];
    bool found = false;
    if (in.is_open()) in.getline(line, 500);
    out << "role,id,failed_attempts,is_locked" << endl;
    while (in.is_open() && in.getline(line, 500)) {
        if (line[0] == '\0') continue;
        char fileRole[20], fileId[20], fileFailed[20], fileLocked[20];
        extractField(line, 0, fileRole);
        extractField(line, 1, fileId);
        extractField(line, 2, fileFailed);
        extractField(line, 3, fileLocked);
        if (stringCompare(fileRole, role) && toInt(fileId) == id) {
            found = true;
            out << role << "," << id << ",0,0" << endl;
        }
        else {
            out << fileRole << "," << toInt(fileId) << "," << toInt(fileFailed) << "," << toInt(fileLocked) << endl;
        }
    }
    if (in.is_open()) in.close();
    out.close();
    remove("account_locks.txt");
    rename("account_locks_temp.txt", "account_locks.txt");
    return found;
}

void FileHandler::dischargePatient(int id, Storage<Patient>& patients, Storage<Appointment>& appointments, Storage<Bill>& bills, Storage<Prescription>& prescriptions) {
    Patient* target = nullptr;
    for (int i = 0; i < patients.size(); i++) {
        if (patients.getAll()[i].getId() == id) {
            target = &patients.getAll()[i];
            break;
        }
    }

    if (target) {
        ofstream disFile("discharged.txt", ios::app);
        disFile << "PATIENT," << target->getId() << "," << target->getName() << "," << target->getAge() << ","
            << target->getGender() << "," << target->getContact() << "," << target->getPassword() << "," << target->getBalance() << endl;
        for (int i = 0; i < appointments.size(); i++) {
            if (appointments.getAll()[i].getpatid() == id) {
                Appointment& a = appointments.getAll()[i];
                disFile << "APPOINTMENT," << a.getappid() << "," << a.getpatid() << "," << a.getdocid() << ","
                    << a.getdate() << "," << a.gettime() << "," << a.getstatus() << endl;
            }
        }
        for (int i = 0; i < bills.size(); i++) {
            if (bills.getAll()[i].getpatid() == id) {
                Bill& b = bills.getAll()[i];
                disFile << "BILL," << b.getbillid() << "," << b.getpatid() << "," << b.getappid() << ","
                    << b.getamount() << "," << b.getstatus() << "," << b.getdate() << endl;
            }
        }
        for (int i = 0; i < prescriptions.size(); i++) {
            if (prescriptions.getAll()[i].getpatid() == id) {
                Prescription& p = prescriptions.getAll()[i];
                disFile << "PRESCRIPTION," << p.getprescid() << "," << p.getappid() << "," << p.getpatid() << ","
                    << p.getdocid() << "," << p.getdate() << "," << p.getmed() << "," << p.getnotes() << endl;
            }
        }
        disFile.close();
    }

    deletePatient(id);

    ofstream appFile("appointments.txt", ios::trunc);
    appFile << "appointment_id,patient_id,doctor_id,date,time_slot,status" << endl;
    for (int i = 0; i < appointments.size(); i++) {
        if (appointments.getAll()[i].getpatid() != id) {
            Appointment& a = appointments.getAll()[i];
            appFile << a.getappid() << "," << a.getpatid() << "," << a.getdocid() << ","
                << a.getdate() << "," << a.gettime() << "," << a.getstatus() << endl;
        }
    }
    appFile.close();

    ofstream billFile("bills.txt", ios::trunc);
    billFile << "bill_id,patient_id,appointment_id,amount,status,date" << endl;
    for (int i = 0; i < bills.size(); i++) {
        if (bills.getAll()[i].getpatid() != id) {
            Bill& b = bills.getAll()[i];
            billFile << b.getbillid() << "," << b.getpatid() << "," << b.getappid() << ","
                << b.getamount() << "," << b.getstatus() << "," << b.getdate() << endl;
        }
    }
    billFile.close();

    ofstream prescFile("prescriptions.txt", ios::trunc);
    prescFile << "prescription_id,appointment_id,patient_id,doctor_id,date,medicines,notes" << endl;
    for (int i = 0; i < prescriptions.size(); i++) {
        if (prescriptions.getAll()[i].getpatid() != id) {
            Prescription& p = prescriptions.getAll()[i];
            prescFile << p.getprescid() << "," << p.getappid() << "," << p.getpatid() << ","
                << p.getdocid() << "," << p.getdate() << "," << p.getmed() << ","
                << p.getnotes() << endl;
        }
    }
    prescFile.close();
}