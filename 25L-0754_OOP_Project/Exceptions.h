#pragma once
class HospitalException {
	char message[200];
public:
	HospitalException(const char* a) {
		int len = 0;
		while (a[len] != '\0'&& len<199) len++;
		for (int i = 0; i < len; i++) {
			message[i] = a[i];			
		}
		message[len] = '\0';
	}
	virtual const char* what() {
		return message;
	}
};
class FileNotFoundException : public HospitalException {
	public:
		FileNotFoundException():HospitalException("File Not Found"){}
};
class InsufficientFundsException : public HospitalException {
	public:
		InsufficientFundsException() :HospitalException("Insufficient Funcs") {}
};
class InvalidInputException :public HospitalException {
	public:
		InvalidInputException():HospitalException("Invalid Input"){}
};
class SlotUnavailableException :public HospitalException {
	public:
		SlotUnavailableException():HospitalException("Slot is unavailable"){}
};
