#pragma once
#include <iostream>

using namespace std;

template<typename T>
class Storage {
private:
    T data[100];
    int count;

public:
    Storage() {
        count = 0;
    }

    bool add(const T& item) {
        if (count < 100) {
            data[count] = item;
            count++;
            return true;
        }
        cout << "Storage Error: Capacity reached (Max 100 entries)." << endl;
        return false;
    }

    bool update(int id, const T& newItem, int (*getID)(const T&)) {
        for (int i = 0; i < count; i++) {
            if (getID(data[i]) == id) {
                data[i] = newItem;
                return true;
            }
        }
        return false;
    }

    void removeById(int id, int (*getID)(const T&)) {
        for (int i = 0; i < count; i++) {
            if (getID(data[i]) == id) {
              
                for (int j = i; j < count - 1; j++) {
                    data[j] = data[j + 1];
                }
                count--;
                return;
            }
        }
    }

    T* findById(int id, int (*getID)(const T&)) {
        for (int i = 0; i < count; i++) {
            if (getID(data[i]) == id) {
                return &data[i];
            }
        }
        return nullptr;
    }

    T* getAll() {
        return data;
    }

    int size() const {
        return count;
    }

    void clear() {
        count = 0;
    }
};