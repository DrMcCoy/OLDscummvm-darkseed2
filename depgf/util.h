#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#include <iostream>

#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

template <typename T> T ABS (T x) { return (x >= 0) ? x : -x; }
template <typename T> T MAX (T a, T b) { return (a > b) ? a : b; }
template <typename T> T MIN (T a, T b) { return (a < b) ? a : b; }
template <typename T> void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

uint8 readUint8(std::ifstream &stream) {
	uint8 x = (uint8) stream.get();
	return x;
}

uint16 readUint16BE(std::ifstream &stream) {
	uint16 u1 = readUint8(stream);
	uint16 u2 = readUint8(stream);

	return (uint16) (u1 << 8) | u2;
}

uint32 readUint32BE(std::ifstream &stream) {
	uint32 u1 = readUint8(stream);
	uint32 u2 = readUint8(stream);
	uint32 u3 = readUint8(stream);
	uint32 u4 = readUint8(stream);

	return (uint32) (u1 << 24) | (u2 << 16) | (u3 << 8) | u4;
}

void readFixedString(std::ifstream &stream, char *str, int n) {
	stream.read(str, n);
	str[n] = '\0';
}

#endif // UTIL_H
