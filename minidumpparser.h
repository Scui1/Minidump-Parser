#include <Windows.h>
#include <DbgHelp.h>
#include <iostream>

namespace MinidumpParser
{
	HANDLE fileHandle{ 0 };
	HANDLE fileMapping{ 0 };
	MINIDUMP_HEADER dumpHeader{ 0 };
	DWORD tebAddress;
	DWORD pebAddress;
	ULONG64 memStartAddress;

	void* mappedDump;

	namespace Private
	{
		void ParseMemory64ListStream(MINIDUMP_DIRECTORY* directory)
		{
			std::cout << "----------MEMORY SHIT----------" << std::endl;
			MINIDUMP_MEMORY64_LIST* memoryList = reinterpret_cast<MINIDUMP_MEMORY64_LIST*>((uintptr_t)mappedDump + directory->Location.Rva);

			memStartAddress = (ULONG64)mappedDump + memoryList->BaseRva;
			for (ULONG64 i = 0; i < memoryList->NumberOfMemoryRanges; i++)
			{
				MINIDUMP_MEMORY_DESCRIPTOR64* currentDescriptor = &memoryList->MemoryRanges[i];
				if (currentDescriptor->StartOfMemoryRange < 0x7FFE0000)
					std::cout << std::dec << i << ": " << "0x" << std::hex << std::uppercase << currentDescriptor->StartOfMemoryRange << ", size: 0x" << currentDescriptor->DataSize << std::endl;

				if (currentDescriptor->StartOfMemoryRange == 0x7C4A0000)
					DebugBreak();

				memStartAddress += currentDescriptor->DataSize;
			}
		}

		void ParseMemoryInfoListStream(MINIDUMP_DIRECTORY* directory)
		{
			std::cout << "----------MEMORY INFO SHIT----------" << std::endl;
			MINIDUMP_MEMORY_INFO_LIST* memoryInfoList = reinterpret_cast<MINIDUMP_MEMORY_INFO_LIST*>((uintptr_t)mappedDump + directory->Location.Rva);

			MINIDUMP_MEMORY_INFO* memoryInfoArray = reinterpret_cast<MINIDUMP_MEMORY_INFO*>((uintptr_t)memoryInfoList + memoryInfoList->SizeOfHeader);
			for (int i = 0; i < memoryInfoList->NumberOfEntries; i++)
			{
				MINIDUMP_MEMORY_INFO* currentMemoryInfo = &memoryInfoArray[i];
				if (currentMemoryInfo->BaseAddress < 0x7FFE0000)
					std::cout << std::dec << i << ": " << "0x" << std::hex << std::uppercase << currentMemoryInfo->BaseAddress << ", size: 0x" << currentMemoryInfo->RegionSize << ", prot: 0x" << currentMemoryInfo->Protect << std::endl;
			}
		}

		void ParseThreadListStream(MINIDUMP_DIRECTORY* directory)
		{
			std::cout << "----------THREAD SHIT----------" << std::endl;
			MINIDUMP_THREAD_LIST* threadList = reinterpret_cast<MINIDUMP_THREAD_LIST*>((uintptr_t)mappedDump + directory->Location.Rva);

			if (threadList->NumberOfThreads > 0)
				tebAddress = threadList->Threads[0].Teb;
		}
	}

	void ReadAtAddress(DWORD address, void* outputBuffer, DWORD outputBufferSize)
	{
		memcpy(outputBuffer, reinterpret_cast<void*>(memStartAddress + address), outputBufferSize);
	}

	bool ParseFile(const char* path)
	{
		fileHandle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		if (!fileHandle)
			return false;

		fileMapping = CreateFileMappingA(fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (!fileMapping)
		{
			CloseHandle(fileHandle);
			return false;
		}

		mappedDump = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0); // numberOfBytesToMap is aligned to pagesize, so we can't use less than 4096
		if (!mappedDump)
		{
			CloseHandle(fileHandle);
			CloseHandle(fileMapping);
			return false;
		}

		MINIDUMP_HEADER* header = reinterpret_cast<MINIDUMP_HEADER*>(mappedDump);
		if (header->Signature != MINIDUMP_SIGNATURE)
			return false;


		MINIDUMP_DIRECTORY* streamDirectories = reinterpret_cast<MINIDUMP_DIRECTORY*>((uintptr_t)mappedDump + header->StreamDirectoryRva);
		for (int i = 0; i < header->NumberOfStreams; i++)
		{
			MINIDUMP_DIRECTORY* currentDirectory = &streamDirectories[i];

			/*const DWORD rva = currentDirectory->Location.Rva;
			const DWORD size = currentDirectory->Location.DataSize;
			std::cout << "Stream nr " << i << ", rva: " << rva << ", size: " << size << ", type: " << currentDirectory->StreamType << std::endl;*/


			if (currentDirectory->StreamType == MINIDUMP_STREAM_TYPE::Memory64ListStream)
				Private::ParseMemory64ListStream(currentDirectory);
			else if (currentDirectory->StreamType == MINIDUMP_STREAM_TYPE::MemoryInfoListStream)
				Private::ParseMemoryInfoListStream(currentDirectory);
			else if (currentDirectory->StreamType == MINIDUMP_STREAM_TYPE::ThreadListStream)
				Private::ParseThreadListStream(currentDirectory);

		}
		/*char buf[2];
		ReadAtAddress(0x7C4A0000, buf, 2);*/

		return true;
	}
}