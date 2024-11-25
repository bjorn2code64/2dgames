#include "wrap32lib.h"

#include <random>

std::default_random_engine l_generator;

void w32seed() {
	l_generator.seed(GetTickCount());
}

DWORD w32rand(DWORD lo, DWORD hi) {
	std::uniform_int_distribution<int> distribution(lo, hi);
	return distribution(l_generator);
}

DWORD w32rand(DWORD hi) {
	return w32rand(0, hi);
}

float w32randf(float lo, float hi) {
	std::uniform_real_distribution<float> distribution(lo, hi);
	return distribution(l_generator);
}

float w32randf(float hi) {
	return w32randf(0, hi);
}

void w32GetError(std::wstring& ret, DWORD dw) {
	wchar_t* buf;
	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&buf, 0, NULL);
	ret = buf;
	ret += L" (";
	ret += std::to_wstring(dw);
	ret += L")";
	::LocalFree(buf);
}

int w32ReplaceStrings(std::string& source, LPCSTR szFrom, LPCSTR szTo) {
	int count = 0;
	size_t fromlen = strlen(szFrom);
	size_t tolen = strlen(szTo);
	size_t index = 0;
	for (;;) {
		/* Locate the substring to replace. */
		index = source.find(szFrom, index);
		if (index == std::string::npos) break;

		/* Make the replacement. */
		source.replace(index, fromlen, szTo);

		/* Advance index forward so the next iteration doesn't pick it up as well. */
		index += tolen;
		count++;
	}

	return count;
}

