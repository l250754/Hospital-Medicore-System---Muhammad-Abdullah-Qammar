#pragma once
class Person {
protected:
	char* name;
	int id;
private:
	char* password;
public:
	Person();
	Person(const char*, int, const char*);
	virtual ~Person();
	Person(const Person& other);		
	Person& operator=(const Person& other);
	int getId() const;
	const char* getPassword() const;	
	virtual void displayMenu() = 0;
	virtual void display() = 0;
	const char* getName() const;
};