#include "BA2.h"



bool BA2::Open(const char* fn)
{
	FILE* fo = fopen(fn, "rb");
	if (fo == NULL)
	{
		return false;
	}
	fread(&header, sizeof(Header), 1, fo);

	printf("numFiles: %d\n", header.numFiles);

	if (memcmp(header.magic, "BTDX", sizeof(header.magic)) != 0)
	{
		return false;
	}
	fileName = string(fn);
	if (memcmp(header.type, "GNRL", sizeof(header.type)) == 0)
	{
		fileRecords.resize(header.numFiles);
		for (int i = 0; i < header.numFiles; i++)
		{
			
			fileRecords[i]=FileRecord(fo);
		}
	}
	else if (memcmp(header.type, "DX10", sizeof(header.type)) == 0){
		//textures
		for (int i = 0; i < header.numFiles; i++)
		{
			texRecords.push_back(TexRecord(fo));
		}

	}
	else{
		return false;
	}

	_fseeki64(fo, header.nameTableOffset, SEEK_SET);
	for (int i = 0; i < header.numFiles; i++)
	{
		uint16 len;
		fread(&len, 2, 1, fo);
		vector<char> fn(len + 1);
		fread(&fn[0], 1, len, fo);
		fn[len] = '\0';
		nameTable.push_back(string(&fn[0]));
	}
	fclose(fo);
	return true;
}





int BA2::Extract(uint32 fid, vector<uchar> &fd)
{
	if (memcmp(header.type, "GNRL", sizeof(header.type)) == 0)
	{
		bool isCompressed = true;
		vector<uchar> comp(fileRecords[fid].entry.packSz);
		if (comp.size() == 0)
		{
			isCompressed = false;
		}
		fd.resize(fileRecords[fid].entry.fullSz);
		FILE* fo = fopen(fileName.c_str(), "rb");
		_fseeki64(fo, fileRecords[fid].entry.offset, SEEK_SET);
		if (isCompressed)
		{

			fread(&comp[0], 1, comp.size(), fo);
			uncompress(&fd[0], &fileRecords[fid].entry.fullSz, &comp[0], comp.size());
		}
		else
		{
			fread(&fd[0], 1, fd.size(), fo);
		}
			fclose(fo);
		
		
		return 1;
	}
	else if (memcmp(header.type, "DX10", sizeof(header.type)) == 0)
	{
		DDS_HEADER ddsHeader={ 0 };
		ddsHeader.dwSize = sizeof(ddsHeader);
		ddsHeader.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_LINEARSIZE | DDS_HEADER_FLAGS_MIPMAP;
		ddsHeader.dwHeight = texRecords[fid].entry.height;
		ddsHeader.dwWidth = texRecords[fid].entry.width;
		ddsHeader.dwMipMapCount = texRecords[fid].entry.numMips;
		ddsHeader.ddspf.dwSize = sizeof(DDS_PIXELFORMAT);
		ddsHeader.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE | DDS_SURFACE_FLAGS_MIPMAP;
		bool ok = true;

		switch (texRecords[fid].entry.format)
		{
		case DXGI_FORMAT_BC1_UNORM:
			ddsHeader.ddspf.dwFlags = DDS_FOURCC;
			ddsHeader.ddspf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '1');
			ddsHeader.dwPitchOrLinearSize = texRecords[fid].entry.width * texRecords[fid].entry.height / 2;	// 4bpp
			break;

		case DXGI_FORMAT_BC2_UNORM:
			ddsHeader.ddspf.dwFlags = DDS_FOURCC;
			ddsHeader.ddspf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '3');
			ddsHeader.dwPitchOrLinearSize = texRecords[fid].entry.width * texRecords[fid].entry.height;	// 8bpp
			break;

		case DXGI_FORMAT_BC3_UNORM:
			ddsHeader.ddspf.dwFlags = DDS_FOURCC;
			ddsHeader.ddspf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '5');
			ddsHeader.dwPitchOrLinearSize = texRecords[fid].entry.width * texRecords[fid].entry.height;	// 8bpp
			break;

		case DXGI_FORMAT_BC5_UNORM:
			ddsHeader.ddspf.dwFlags = DDS_FOURCC;
			//if (m_useATIFourCC)
				ddsHeader.ddspf.dwFourCC = MAKEFOURCC('A', 'T', 'I', '2');	// this is more correct but the only thing I have found that supports it is the nvidia photoshop plugin
			//else
				//ddsHeader.ddspf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '5');

				ddsHeader.dwPitchOrLinearSize = texRecords[fid].entry.width * texRecords[fid].entry.height;	// 8bpp
			break;

		case DXGI_FORMAT_BC7_UNORM:
			// totally wrong but not worth writing out the DX10 header
			ddsHeader.ddspf.dwFlags = DDS_FOURCC;
			ddsHeader.ddspf.dwFourCC = MAKEFOURCC('B', 'C', '7', '\0');
			ddsHeader.dwPitchOrLinearSize = texRecords[fid].entry.width * texRecords[fid].entry.height;	// 8bpp
			break;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
			ddsHeader.ddspf.dwFlags = DDS_RGBA;
			ddsHeader.ddspf.dwRGBBitCount = 32;
			ddsHeader.ddspf.dwRBitMask = 0x00FF0000;
			ddsHeader.ddspf.dwGBitMask = 0x0000FF00;
			ddsHeader.ddspf.dwBBitMask = 0x000000FF;
			ddsHeader.ddspf.dwABitMask = 0xFF000000;
			ddsHeader.dwPitchOrLinearSize = texRecords[fid].entry.width * texRecords[fid].entry.height * 4;	// 32bpp
			break;

		case DXGI_FORMAT_R8_UNORM:
			ddsHeader.ddspf.dwFlags = DDS_RGB;
			ddsHeader.ddspf.dwRGBBitCount = 8;
			ddsHeader.ddspf.dwRBitMask = 0xFF;
			ddsHeader.dwPitchOrLinearSize = texRecords[fid].entry.width * texRecords[fid].entry.height;	// 8bpp
			break;

		default:
			return 0;
			ok = false;
			break;
		}
		if (ok)
		{
			//get size
			uint32 size = 4 + sizeof(ddsHeader);
			fd.resize(size);
			uint32 magic = DDS_MAGIC;
			FILE* fo = fopen(fileName.c_str(), "rb");
			memcpy(&fd[0], &magic, 4);
			memcpy(&fd[0] + 4, &ddsHeader, sizeof(ddsHeader));
			for (int i = 0; i < texRecords[fid].entry.numChunks;i++)
			{
				fd.resize(size + texRecords[fid].chunks[i].fullSz);
				vector<uchar> comp(texRecords[fid].chunks[i].packSz);
				vector<uchar> full(texRecords[fid].chunks[i].fullSz);
				_fseeki64(fo, texRecords[fid].chunks[i].offset, SEEK_SET);
				bool isCompressed = true;
				if (texRecords[fid].chunks[i].packSz == 0)
				{
					isCompressed = false;
				}

				if (isCompressed)
				{
					fread(&comp[0], 1, comp.size(), fo);
					uncompress(&full[0], &texRecords[fid].chunks[i].fullSz, &comp[0], texRecords[fid].chunks[i].packSz);
				}
				else{
					fread(&full[0], 1, full.size(), fo);
				}
				memcpy(&fd[0] + size, &full[0], texRecords[fid].chunks[i].fullSz);
				size += texRecords[fid].chunks[i].fullSz;
			}
			fclose(fo);
			
		}


		return 1;
	}
	else{
		return 0;
	}
}

BA2::BA2()
{
}


BA2::~BA2()
{
}
