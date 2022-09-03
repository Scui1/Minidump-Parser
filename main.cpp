#include "minidumpparser.h"
//#include <minidumpapiset.h>

void Dumper()
{
	constexpr DWORD processId = 13284;
	HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (!procHandle)
		return;

	HANDLE dumpFile = CreateFileA("B:\\temp\\csgodump1.dmp", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
	if (!dumpFile)
		return;

	/*constexpr auto dumpType = MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo;
	if (!MiniDumpWriteDump(procHandle, processId, dumpFile, (MINIDUMP_TYPE)dumpType, nullptr, nullptr, nullptr))
		return;*/
}

int main() 
{
	//Dumper();
	const char* a = "B:\\temp\\csgodump1.dmp";
	const char* b = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\skeetcheat.mdmp";
	MinidumpParser::ParseFile(a);

	return 1;
}