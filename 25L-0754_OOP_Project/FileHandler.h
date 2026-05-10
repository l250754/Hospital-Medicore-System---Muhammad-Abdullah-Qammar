#pragma once
#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"
#include <iostream>
using namespace std;

class FileHandler {
public:
    static void initializeFiles();
    // Loading functions
    static void loadPatients(Storage<Patient>& storage);
    static void loadDoctors(Storage<Doctor>& storage);
    static void loadAdmin(Admin& admin);
    static void loadAppointments(Storage<Appointment>& storage);
    static void loadBills(Storage<Bill>& storage);
    static void loadPrescriptions(Storage<Prescription>& storage);

    // Appending functions
    static void appendPatient(const Patient& p);
    static void appendDoctor(const Doctor& d);
    static void appendAppointment(const Appointment& a);
    static void appendBill(const Bill& b);
    static void appendPrescription(const Prescription& p);

    // Updating functions
    static void updatePatient(const Patient& p);
    static void updateAppointment(const Appointment& a);
    static void updateBill(const Bill& b);

    // Deleting functions
    static void deleteDoctor(int id);
    static void deletePatient(int id);

    // Other
    static void logSecurity(const char* role, int id, const char* result);
    static void dischargePatient(int id, Storage<Patient>& patients, Storage<Appointment>& appointments, Storage<Bill>& bills, Storage<Prescription>& prescriptions);
    static int getMaxId(const char* filename);
    static bool idExists(const char* filename, int id);
    static int getNextUniqueId(const char* filename);
    static bool isAccountLocked(const char* role, int id);
    static void recordFailedLogin(const char* role, int id);
    static void resetFailedLogin(const char* role, int id);
    static bool unlockAccount(const char* role, int id);
};