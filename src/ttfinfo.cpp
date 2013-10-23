
#include <ttfinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define SAFE_FREE(x) if (x != NULL) { free(x); x = NULL; }

static int _TTFinfo_analyze_OffsetTable(TTFinfo* info) {
	char* cur = (char*)info->_ttfFile;
	OffsetTable* ot = &info->offsetTable;
	memcpy(ot, cur, sizeof(OffsetTable));
	TTFinfo_reverseByteOrder(&ot->fixed, sizeof(TTFinfo_Fixed));
	TTFinfo_reverseByteOrder(&ot->numTables, sizeof(TTFinfo_USHORT));
	TTFinfo_reverseByteOrder(&ot->searchRange, sizeof(TTFinfo_USHORT));
	TTFinfo_reverseByteOrder(&ot->entrySelector, sizeof(TTFinfo_USHORT));
	TTFinfo_reverseByteOrder(&ot->rangeShift, sizeof(TTFinfo_USHORT));

#ifdef TTFINFO_DEBUG
	printf("# Offset table\n");
	printf("---------------------------------------\n");
	printf("TTFinfo_Fixed\t\t\t: %d\n", ot->fixed);
	printf("numTables\t\t: %d\n", ot->numTables);
	printf("searchRange\t\t: %d\n", ot->searchRange);
	printf("entrySelector\t\t: %d\n", ot->entrySelector);
	printf("rangeShift\t\t: %d\n", ot->rangeShift);
	printf("\n");
#endif
	// テーブル数分のデータがないならエラー
	if (info->_ttfFileSize < sizeof(OffsetTable)+sizeof(TableDirectoryEntry)*info->offsetTable.numTables) {
		return -1;
	}
	return 0;
}

static int _TTFinfo_analyze_TableDirectoryEntry(TTFinfo* info) {
	int i;
	TableDirectoryEntry* tde;
	char* cur = (char*)info->_ttfFile + sizeof(OffsetTable);

	info->tableDirectoryEntries = (TableDirectoryEntry*)malloc(sizeof(TableDirectoryEntry)*info->offsetTable.numTables);
	for (i = 0; i<info->offsetTable.numTables; i++) {
		tde = &info->tableDirectoryEntries[i];
		memcpy(tde, cur, sizeof(TableDirectoryEntry));
		TTFinfo_reverseByteOrder(&tde->checkSum, sizeof(TTFinfo_ULONG));
		TTFinfo_reverseByteOrder(&tde->offset, sizeof(TTFinfo_ULONG));
		TTFinfo_reverseByteOrder(&tde->length, sizeof(TTFinfo_ULONG));
		cur += sizeof(TableDirectoryEntry);
	}

#ifdef TTFINFO_DEBUG
	printf("# Table directory entry\n");
	printf("TAG\t  CHECK_SUM\tOFFSET\tLENGTH\n");
	printf("---------------------------------------\n");
	for (i = 0; i < info->offsetTable.numTables; i++) {
		tde = &info->tableDirectoryEntries[i];
		printf("%c%c%c%c\t: 0x%08x\t%d\t%d\n",
			tde->tag[0], tde->tag[1], tde->tag[2], tde->tag[3],
			tde->checkSum,
			tde->offset,
			tde->length);

		// テーブルオフセットの保存
		// テーブル内のデータは、テーブル先頭を基準とするので取得しておく
		if (tde->tag[0] == 'n' && tde->tag[1] == 'a' && tde->tag[2] == 'm' && tde->tag[3] == 'e') {
			info->_tableOffset_name = tde->offset;// とりあえずnameテーブルだけ。
		}
	}
	printf("\n");
#endif
	return 0;
}

void TTFinfo_reverseByteOrder(void* data, int n) {
	int i;
	char t;
	char* p = (char*)data;
	for (i = 0; i<n >> 1; i++) {
		t = *(p + i);
		*(p + i) = *(p + n - i - 1);
		*(p + n - i - 1) = t;
	}
}

int TTFinfo_init(TTFinfo* info, const void* ttf, size_t ttf_size) {
	memset(info, 0x00, sizeof(TTFinfo));
	info->_ttfFile = ttf;
	info->_ttfFileSize = ttf_size;

	if (_TTFinfo_analyze_OffsetTable(info) < 0) {
		return -1;
	}
	if (_TTFinfo_analyze_TableDirectoryEntry(info) < 0) {
		return -1;
	}
	return 0;
}

int TTFinfo_analyzeNameTable(TTFinfo* info) {
	char* cur = (char*)info->_ttfFile + info->_tableOffset_name;
	NameTable* nt = &info->nameTable;
	memcpy(nt, cur, sizeof(NameTable));
	TTFinfo_reverseByteOrder(&nt->format, sizeof(TTFinfo_USHORT));
	TTFinfo_reverseByteOrder(&nt->count, sizeof(TTFinfo_USHORT));
	TTFinfo_reverseByteOrder(&nt->stringOffset, sizeof(TTFinfo_USHORT));

#ifdef TTFINFO_DEBUG
	printf("# Name table\n");
	printf("---------------------------------------\n");
	printf("format\t\t\t: %d\n", nt->format);
	printf("count\t\t\t: %d\n", nt->count);
	printf("stringOffset\t\t: %d\n", nt->stringOffset);
	printf("\n");
#endif
	return 0;
}

int TTFinfo_analyzeNameRecords(TTFinfo* info) {
	int i;
	NameRecord* nr;
	char* cur = (char*)info->_ttfFile + info->_tableOffset_name;
	cur += sizeof(NameTable);// nameレコードの配列の先頭へ
	info->nameRecords = (NameRecord*)malloc(sizeof(NameRecord)*info->nameTable.count);
	for (i = 0; i < info->nameTable.count; i++) {
		nr = &info->nameRecords[i];
		memcpy(nr, cur, sizeof(NameRecord));
		TTFinfo_reverseByteOrder(&nr->platformID, sizeof(TTFinfo_USHORT));
		TTFinfo_reverseByteOrder(&nr->encodingID, sizeof(TTFinfo_USHORT));
		TTFinfo_reverseByteOrder(&nr->languageID, sizeof(TTFinfo_USHORT));
		TTFinfo_reverseByteOrder(&nr->nameID, sizeof(TTFinfo_USHORT));
		TTFinfo_reverseByteOrder(&nr->length, sizeof(TTFinfo_USHORT));
		TTFinfo_reverseByteOrder(&nr->offset, sizeof(TTFinfo_USHORT));
		cur += sizeof(NameRecord);
	}

#ifdef TTFINFO_DEBUG
	printf("# Name records\n");
	printf("NO | PL\tENC\tLANG\tNAME\tLEN\tOFFSET\n");
	printf("---------------------------------------\n");
	for (i = 0; i < info->nameTable.count; i++) {
		nr = &info->nameRecords[i];
		printf("%02d | %d\t%d\t%d\t%d\t%d\t0x%08x\n",
			i+1,
			nr->platformID,
			nr->encodingID,
			nr->languageID,
			nr->nameID,
			nr->length,
			nr->offset);
	}
#endif
	return 0;
}

int TTFinfo_getNameData(TTFinfo* info, int pid, int nid, int lid, void** dst, size_t* dstSize) {
	int i;
	NameRecord* nr;
	char* pt = (char*)info->_ttfFile + info->_tableOffset_name + info->nameTable.stringOffset;
	for (i = 0; i < info->nameTable.count; i++) {
		nr = &info->nameRecords[i];
		if (nr->platformID == pid && nr->nameID == nid && nr->languageID == lid) {
			*dst = pt + nr->offset;
			*dstSize = nr->length;
			return 0;
		}
	}
	return -1;
}

void TTFinfo_term(TTFinfo* info) {
	SAFE_FREE(info->tableDirectoryEntries);
	SAFE_FREE(info->nameRecords);
}
