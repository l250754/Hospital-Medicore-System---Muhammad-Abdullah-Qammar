#define _CRT_SECURE_NO_WARNINGS
#include "Validator.h"
#include <ctime>

bool Validator::validateDate(const char* date) {
   
    if (date[0] == '\0') return false;
   
    if (date[2] != '-' || date[5] != '-') return false;
   
    int day = (date[0] - '0') * 10 + (date[1] - '0');
   
    int month = (date[3] - '0') * 10 + (date[4] - '0');
   
    int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 + (date[8] - '0') * 10 + (date[9] - '0');
    
    time_t now = time(nullptr);
    tm* local = localtime(&now);
    int currentYear = local->tm_year + 1900;

    if (day < 1 || day > 31) return false;
    if (month < 1 || month > 12) return false;
    if (year < currentYear) return false;

    return true;
}

bool Validator::validateTimeSlot(const char* time) {
   
    const char* slots[8] = { "09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00" };

    for (int i = 0; i < 8; i++) {
        int j = 0;
        while (slots[i][j] != '\0' && time[j] != '\0') {
            if (slots[i][j] != time[j]) break;
            j++;
        }
        if (slots[i][j] == '\0' && time[j] == '\0') {
            return true;
        }
    }
    return false;
}

bool Validator::validateContact(const char* contact) {
  
    int len = 0;
    while (contact[len] != '\0') len++;
    if (len != 11) return false;

    for (int i = 0; i < len; i++) {
        if (contact[i] < '0' || contact[i] > '9') return false;
    }
    return true;
}

bool Validator::validatePassword(const char* password) {
    int len = 0;
    while (password[len] != '\0') len++;
    return (len >= 6);
}

bool Validator::validatePositiveFloat(float amount) {
    return (amount > 0);
}

bool Validator::validateMenuChoice(int choice, int min, int max) {
    return (choice >= min && choice <= max);
}

bool Validator::validateID(int id) {
    return (id > 0);
}