# Hospital-Medicore-System---Muhammad-Abdullah-Qammar 25L-0754 BCS-2B|

# MediCore Hospital Management System

## Project
Hospital Management System with SFML graphics and file-based storage.

## Features
- **Patient**: Book/cancel appointments, view records/bills, pay bills, top up balance
- **Doctor**: View today's appointments, mark complete/no-show, write prescriptions, view patient history
- **Admin**: Add/remove doctors, view all data, discharge patients, generate reports, unlock accounts

## Technologies
- C++17
- SFML 2.6.1 (graphics)
- CSV files for data storage (no database)

## Classes
Person, Patient, Doctor, Admin, Appointment, Bill, Prescription, Storage<T>, FileHandler, Validator, Exceptions

## Files
All .h and .cpp files in root directory. Data files are generated automatically.

## How to Run
1. Open solution in Visual Studio 2022
2. Ensure SFML 2.6.1 is properly configured
3. Build and run

## Login Credentials (Default)
- **Admin**: ID `1`, Password `admin123`
- **Patient**: Register new or use existing in patients.txt
- **Doctor**: Add via admin or use existing in doctors.txt

## Roll Number
25L-0754
