#pragma once
class Patient;
class Doctor;

bool stringCompare(const char* s1, const char* s2);
int toInt(const char* str);
float toFloat(const char* str);
void extractField(const char* line, int fieldIndex, char* output);
void stringCopy(char* dest, const char* src);

int getPatId(const Patient& p);
int getDocId(const Doctor& d);

/* Dynamic growable C-string buffer (assignment rule: no std::string) */
struct DynBuf {
	char* data;
	int capacity;
	int length;
};

void dynBufInit(DynBuf* b);
void dynBufFree(DynBuf* b);
void dynBufClear(DynBuf* b);
void dynBufAppendStr(DynBuf* b, const char* s);
void dynBufAppendInt(DynBuf* b, int v);
void dynBufAppendFloat2(DynBuf* b, float f);
const char* dynBufCStr(const DynBuf* b);