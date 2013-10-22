
#include <stdio.h>
#include <locale.h>
#include <string>
#include <vector>

#include "ttfinfo.h"

int loadFile(char* filepath, void** dst, size_t* osz) {
	void* buf = NULL;
	FILE* fp = NULL;
	fpos_t sz = 0;
	size_t szr = 0;

	fp = fopen(filepath, "rb");
	if (fp == NULL) return -1;
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &sz);
	buf = malloc(sz);
	fseek(fp, 0, SEEK_SET);
	szr = fread(buf, sizeof(char), sz, fp);
	fclose(fp);
	
	*dst = buf;
	*osz = szr;
	return 0;
}

void w2mb(const std::wstring &src, std::string &dest) {
	char *mbs = new char[src.length() * MB_CUR_MAX + 1];
	wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
	dest = mbs;
	delete[] mbs;
}

int main() {
	void* ttf;
	size_t ttf_size;
	setlocale(LC_CTYPE, "");

	if (loadFile(">>>>>>>>>>>>> TTF FILE PATH <<<<<<<<<<<<<<<", &ttf, &ttf_size) < 0) {
		return -1;
	}

	TTFinfo TTFinfo;
	TTFinfo_init(&TTFinfo, ttf, ttf_size);
	TTFinfo_analyzeNameTable(&TTFinfo);
	TTFinfo_analyzeNameRecords(&TTFinfo);

	void* data;
	size_t dataSize;
	if (TTFinfo_getNameData(&TTFinfo, TTFinfo_MAC, TTFinfo_FULL_FONT_NAME, TTFinfo_ENG, &data, &dataSize) >= 0) {
		std::vector<char> v(dataSize);
		memcpy(&v[0], data, dataSize);
		v.push_back('\0');
		printf("name : %s\n", &v[0]);
	}

	if (TTFinfo_getNameData(&TTFinfo, TTFinfo_WIN, TTFinfo_FULL_FONT_NAME, TTFinfo_JPN, &data, &dataSize) >= 0) {
		std::vector<short> v;
		v.assign(((dataSize+1)/2), 0);
		memcpy(&v[0], data, dataSize);
		for (int j = 0; j<v.size(); j++) {
			TTFinfo_reverseByteOrder(&v[j], sizeof(short));
		}
		v.push_back('\0');
		std::wstring wname = reinterpret_cast<wchar_t*>(&v[0]);
		std::string name;
		w2mb(wname, name);
		printf("name : %s\n", name.c_str());
	}
	
	TTFinfo_term(&TTFinfo);
	free(ttf);

	while (1);
	return 0;
}
