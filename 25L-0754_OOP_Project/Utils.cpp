#include "Utils.h"
#include "Patient.h"
#include "Doctor.h"

static int cStrLenLocal(const char* s) {
	int n = 0;
	if (!s) return 0;
	while (s[n] != '\0') n++;
	return n;
}

static void dynBufEnsure(DynBuf* b, int extra) {
	int need = b->length + extra + 1;
	if (b->data != nullptr && need <= b->capacity) return;
	int newCap = (b->capacity == 0) ? 2048 : b->capacity;
	while (newCap < need) newCap *= 2;
	char* nbuf = new char[newCap];
	if (b->data != nullptr) {
		for (int i = 0; i < b->length; i++) nbuf[i] = b->data[i];
		delete[] b->data;
	}
	b->data = nbuf;
	b->capacity = newCap;
}

void dynBufInit(DynBuf* b) {
	b->data = nullptr;
	b->capacity = 0;
	b->length = 0;
}

void dynBufFree(DynBuf* b) {
	if (b->data != nullptr) delete[] b->data;
	b->data = nullptr;
	b->capacity = 0;
	b->length = 0;
}

void dynBufClear(DynBuf* b) {
	b->length = 0;
	if (b->data != nullptr) b->data[0] = '\0';
}

void dynBufAppendStr(DynBuf* b, const char* s) {
	if (s == nullptr) return;
	int n = cStrLenLocal(s);
	dynBufEnsure(b, n);
	for (int i = 0; i < n; i++) b->data[b->length++] = s[i];
	b->data[b->length] = '\0';
}

void dynBufAppendInt(DynBuf* b, int v) {
	if (v == 0) {
		dynBufAppendStr(b, "0");
		return;
	}
	if (v < 0) {
		dynBufAppendStr(b, "-");
		if (v == -2147483647 - 1) {
			dynBufAppendStr(b, "2147483648");
			return;
		}
		v = -v;
	}
	char tmp[16];
	int idx = 0;
	while (v > 0) {
		tmp[idx++] = (char)('0' + (v % 10));
		v /= 10;
	}
	while (idx > 0) {
		char one[2];
		one[0] = tmp[--idx];
		one[1] = '\0';
		dynBufAppendStr(b, one);
	}
}

void dynBufAppendFloat2(DynBuf* b, float f) {
	int whole = (int)f;
	float frac = f - (float)whole;
	if (frac < 0.0f) frac = -frac;
	int cents = (int)(frac * 100.0f + 0.5f);
	if (cents >= 100) {
		whole += cents / 100;
		cents %= 100;
	}
	dynBufAppendInt(b, whole);
	dynBufAppendStr(b, ".");
	if (cents < 10) dynBufAppendStr(b, "0");
	dynBufAppendInt(b, cents);
}

const char* dynBufCStr(const DynBuf* b) {
	if (b->data == nullptr) return "";
	return b->data;
}

bool stringCompare(const char* s1, const char* s2) {
    if (!s1 || !s2) return false;
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return false;
        i++;
    }
    return s1[i] == s2[i];
}

int toInt(const char* str) {
    int result = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9')
            result = result * 10 + (str[i] - '0');
    }
    return result;
}

float toFloat(const char* str) {
    float result = 0;
    float decimal = 0.1f;
    bool afterDot = false;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '.') { afterDot = true; continue; }
        if (str[i] >= '0' && str[i] <= '9') {
            if (!afterDot) result = result * 10 + (str[i] - '0');
            else { result += (str[i] - '0') * decimal; decimal *= 0.1f; }
        }
    }
    return result;
}

void extractField(const char* line, int fieldIndex, char* output) {
    int currentField = 0;
    int pos = 0;
    int outPos = 0;
    while (line[pos] != '\0' && currentField <= fieldIndex) {
        if (line[pos] == ',') currentField++;
        else if (currentField == fieldIndex) output[outPos++] = line[pos];
        pos++;
    }
    output[outPos] = '\0';
}

void stringCopy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int getPatId(const Patient& p) {
    return p.getId();
}

int getDocId(const Doctor& d) {
    return d.getId();
}