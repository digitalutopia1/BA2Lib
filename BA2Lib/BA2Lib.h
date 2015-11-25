// BA2Lib.h

#pragma once
#include "BA2.h";
#include <vector>;

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::IO;

namespace BA2Lib {
	public ref class BA2NET{
	public:
		BA2 *b;
		String^ FileName;
		bool Open(String^ fn)
		{
			
			char* fnc = (char*)(void*)Marshal::StringToHGlobalAnsi(fn);
			bool res = b->Open(fnc);
			this->FileName = Path::GetFileName(fn);
			return res;
		}

		array<String^>^ GetNameTable()
		{
			vector<string> ntc = b->getNameTable();
			array<String^>^ nt = gcnew array<String^>(ntc.size());
			for (int i = 0; i < ntc.size(); i++)
			{
				nt[i] = gcnew String(ntc[i].c_str());
			}
			return nt;
		}

		array<System::Byte>^ Extract(unsigned int index)
		{
			vector<uchar> fdc;
			int ret = b->Extract(index, fdc);
			array<System::Byte>^ fd = gcnew array<System::Byte>(fdc.size());
			for (int i = 0; i < fd->Length; i++)
			{
				fd[i] = fdc[i];
			}
			return fd;
		}
		BA2NET(){ this->b = new BA2(); }
		~BA2NET(){ delete b; };
	};
}
