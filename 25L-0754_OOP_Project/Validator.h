#pragma once

class Validator {
public:
    static bool validateDate(const char* date);
    static bool validateTimeSlot(const char* time);
    static bool validateContact(const char* contact);
    static bool validatePassword(const char* password);
    static bool validatePositiveFloat(float amount);
    static bool validateMenuChoice(int choice, int min, int max);
    static bool validateID(int id);
};