##BA2Lib

A .NET CLI/C++ Wrapper around a C++ class for opening and extracting files from Bethesda's BA2 archive format. The Visual Studio solution/project included in this release, is for the CLI .dll, but the code itself can easily be used to create a standard C++ static or dynamic library. 

###API

```c++
//CLI
public bool Open(String^ ba2Filename)
//C++
public bool Open(const char* ba2Filename)
```
Opens a BA2 file, referenced by `ba2Filename`, and parses the header and name table.

```c++
//CLI
public array<String^>^ GetNameTable()
//C++
vector<string> getNameTable()
```
Returns a copy of the opened BA2 file's name table - which contains the folder and file names of all files within the archive. 

```c++
//CLI
public array<Byte>^ Extract(unsigned int index)
//C++
public int Extract(unsigned long fileIndex, vector<unsigned char> &fileData);
```
In both versions, `Extract()` will retrieve the internal file referenced to by fileIndex, which corresponds to the order the file appears in the name table. In CLI, it will return a Byte array, in the C++ base class, it requires a unsigned char vector passed by reference. 

##Disclaimer
This is not a shining example of a well formatted, if even coded, release-quality library. There lines commented out, methods that are never used, and error handling is practically non-existent. Yet, it works for what I needed it to do. So why release it? Well...

##Credits
This code is based on, and uses small portions of, the source code for Ba2extract, by Ian Patterson of the F4SE team. By making this source available, I'm fulfilling my obligation (via the licensing) to make any source available that is derived from  the above mentioned project. 