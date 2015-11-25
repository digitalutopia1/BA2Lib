#pragma once
#include <vector>
#include <stdio.h>
#include <fstream>
#include <string>
#include "zlib.h"
#include "dds.h"

using namespace std;

typedef unsigned char		uchar;		//!< An unsigned 8-bit integer value
typedef unsigned short		uint16;		//!< An unsigned 16-bit integer value
typedef unsigned long		uint32;		//!< An unsigned 32-bit integer value
typedef unsigned long long	uint64;		//!< An unsigned 64-bit integer value
typedef signed char			int8;		//!< A signed 8-bit integer value
typedef signed short		int16;		//!< A signed 16-bit integer value
typedef signed long			int32;		//!< A signed 32-bit integer value
typedef signed long long	int64;		//!< A signed 64-bit integer value
typedef float				float32;	//!< A 32-bit floating point value
typedef double				float64;	//!< A 64-bit floating point value


enum Errors
{
	ERR_UNK = 0, //because we want 1 = ok
	ERR_OK,
	ERR_FILE_NOT_FOUND, 
	ERR_NOT_BA2, //magic is incorrect
	ERR_MALFORMED_FILE, //file is shorter than nametable offset
};

class BA2
{
public:
	BA2();
	~BA2();

	bool Open(const char* fn);
	int Extract(uint32 fid, vector<uchar> &fd);
	vector<string> getNameTable()
	{
		return nameTable;
	}
	string fileName = "";

	vector<string> buildNameTable(const char* fn, uint64 offset);
	vector<string> nameTable;
#pragma pack(2)	
	struct Header
	{
		char magic[4];
		uint32 version;
		char type[4];
		uint32 numFiles;
		uint64 nameTableOffset;
	}header;
private:
#pragma pack(2)
	struct FileEntry
	{
		uint32 nameHash;
		char ext[4];
		uint32 dirHash;
		uint32 flags;
		uint64 offset;
		uint32 packSz;
		uint32 fullSz;
		uint32 align;
	};

	struct FileRecord
	{
		FileEntry entry;

		FileRecord(){}
		FileRecord(FILE* fo)
		{
			fread(&entry, sizeof(FileEntry), 1, fo);
		}

	};


#pragma pack(2)
	struct TexEntry
	{
		uint32 nameHash;
		char ext[4];
		uint32 dirHash;
		uchar unk8;
		uchar numChunks;
		uint16 chunkHdrLen;
		uint16 height;
		uint16 width;
		uchar numMips;
		uchar format;
		uint16 unk16;
	};
#pragma pack(2)
	struct TexChunk
	{
		uint64 offset;
		uint32 packSz;
		uint32 fullSz;
		uint16 startMip;
		uint16 endMip;
		uint32 align;
	};

	struct TexRecord
	{
		TexEntry entry;
		vector<TexChunk> chunks;
		TexRecord(){}
		TexRecord(FILE* fo)
		{
			fread(&entry, sizeof(TexEntry), 1, fo);
			chunks.resize(entry.numChunks);
			fread(&chunks[0], sizeof(TexChunk), entry.numChunks, fo);
		}
	};

	vector<FileRecord> fileRecords;
	vector<TexRecord> texRecords;

};

