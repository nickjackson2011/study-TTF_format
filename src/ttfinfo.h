////////////////////////////////////////////////////////////////////
//
// ttfinfo.h
//
////////////////////////////////////////////////////////////////////
/*
  
>> acknowledgment
  
  [referenced documents]

  Microsoft typography > OpenType Specification v.1.6 - The OpenType font file
  http://www.microsoft.com/typography/otspec/name.htm

  vanillaの日記
  http://vanillasky-room.cocolog-nifty.com/blog/2008/02/opentype-e54f.html
  http://vanillasky-room.cocolog-nifty.com/blog/2008/02/opentype3name-a.html

  ADOBE TechNote - Fixed型について
  http://kb2.adobe.com/jp/cps/510/510318.html

>> license

   NYSL
  
*/
#ifndef INCLUDED_TTFINFO_H
#define INCLUDED_TTFINFO_H

#define TTFINFO_DEBUG

#include <stdint.h>

typedef uint32_t TTFinfo_Fixed;
typedef uint16_t TTFinfo_USHORT;
typedef uint32_t TTFinfo_ULONG;

#pragma pack(1)
typedef struct _OffsetTable {
	TTFinfo_Fixed fixed;
	TTFinfo_USHORT numTables;
	TTFinfo_USHORT searchRange;
	TTFinfo_USHORT entrySelector;
	TTFinfo_USHORT rangeShift;
}OffsetTable;

typedef struct _TableDirectoryEntry {
	uint8_t tag[sizeof(TTFinfo_ULONG)];
	TTFinfo_ULONG checkSum;
	TTFinfo_ULONG offset;
	TTFinfo_ULONG length;
}TableDirectoryEntry;

// tables ------------------------
typedef struct _NameTable {
	TTFinfo_USHORT format;
	TTFinfo_USHORT count;
	TTFinfo_USHORT stringOffset;
}NameTable;
// -------------------------------

typedef struct _NameRecord {
	TTFinfo_USHORT platformID;
	TTFinfo_USHORT encodingID;
	TTFinfo_USHORT languageID;
	TTFinfo_USHORT nameID;
	TTFinfo_USHORT length;
	TTFinfo_USHORT offset;
}NameRecord;
#pragma pack()

typedef enum {
	TTFinfo_UNICODE,
	TTFinfo_MAC,
	TTFinfo_ISO,
	TTFinfo_WIN,
	TTFinfo_CUSTOM
}TTFinfo_PratformID;

typedef enum {
	TTFinfo_COPYRIGHT,
	TTFinfo_FONT_FAMILY,
	TTFinfo_FONT_SUBFAMILY,
	TTFinfo_FONT_UID,
	TTFinfo_FULL_FONT_NAME,
	TTFinfo_VERSION,
	TTFinfo_POSTSCRIPT_NAME,
	TTFinfo_TRADEMARK,
	TTFinfo_MANUFACTURE_NAME
	//...
}TTFinfo_NameID;

typedef enum {
	TTFinfo_ENG = 0x0000,// Language ID for Windows
	TTFinfo_JPN = 0x0411,
}TTFinfo_LangID;

typedef struct _TTFinfo {
	OffsetTable offsetTable;
	TableDirectoryEntry* tableDirectoryEntries;
	NameTable nameTable;
	NameRecord* nameRecords;

	const void* _ttfFile;
	size_t _ttfFileSize;
	uint32_t _tableOffset_name;

}TTFinfo;

void TTFinfo_reverseByteOrder(void* data, int n);
int TTFinfo_init(TTFinfo* info, const void* ttf, size_t ttf_size);
int TTFinfo_analyzeNameTable(TTFinfo* info);
int TTFinfo_analyzeNameRecords(TTFinfo* info);
int TTFinfo_getNameData(TTFinfo* info, int pratformID, int nameID, int langID, void** dst, size_t* dstSize);
void TTFinfo_term(TTFinfo*);

#endif// INCLUDED_TTFINFO_H
