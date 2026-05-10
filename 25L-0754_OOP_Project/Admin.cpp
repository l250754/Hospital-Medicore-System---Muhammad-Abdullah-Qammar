#include "Admin.h"
#include <iostream>
using namespace std;

Admin::Admin() : Person() {}
Admin::Admin(const char*name, int id, const char* password):Person(name,id,password){}
Admin::Admin(const Admin& other) : Person(other) {}
void Admin::displayMenu() {
    cout << "1. Add Doctor" << endl;
    cout << "2. Remove Doctor" << endl;
    cout << "3. View All Patients" << endl;
    cout << "4. View All Doctors" << endl;
    cout << "5. View All Appointments" << endl;
    cout << "6. View Unpaid Bills" << endl;
    cout << "7. Discharge Patient" << endl;
    cout << "8. View Security Log" << endl;
    cout << "9. Generate Daily Report" << endl;
    cout << "10. Logout" << endl;
}
void Admin::display() {
    cout << "Admin ID: " << id << " Name: " << name << endl;
}
Admin::~Admin() {}