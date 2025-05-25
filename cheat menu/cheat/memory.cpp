#include "memory.h"


void mem::GetWinHandle(HWND &hWndBuffer)
{
	HWND tempHWnd = FindWindow(NULL, _T("Idle Slayer"));
	hWndBuffer = tempHWnd;
}

DWORD mem::GetProcID(HWND hWnd)
{
	DWORD procID{ 0 };

	GetWindowThreadProcessId(hWnd, &procID);

	return procID;
}

DWORD mem::GetHandleCount(HANDLE hProcess)
{
	DWORD handleCount = 0;
	if ( GetProcessHandleCount(hProcess, &handleCount) )
	{
		return handleCount;
	}
	return 0;
}

DWORD mem::GetThreadCount(DWORD processID)
{
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if ( hThreadSnap == INVALID_HANDLE_VALUE ) return 0;

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);
	DWORD threadCount = 0;

	if ( Thread32First(hThreadSnap, &te32) )
	{
		do
		{
			if ( te32.th32OwnerProcessID == processID )
				threadCount++;
		} while ( Thread32Next(hThreadSnap, &te32) );
	}
	CloseHandle(hThreadSnap);
	return threadCount;
}

uintptr_t mem::GetModuleAddress(DWORD ProcID, const TCHAR* moduleName)
{
	uintptr_t address{ 0x0 };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, ProcID);

	if ( hSnapshot != INVALID_HANDLE_VALUE )
	{
		MODULEENTRY32 me32{};
		me32.dwSize = sizeof(MODULEENTRY32);

		if ( Module32First(hSnapshot, &me32) )
		{
			do
			{
				if ( !_tcsicmp(me32.szModule, moduleName) )
				{
					address = (uintptr_t)me32.modBaseAddr;
					break;
				}
			} while ( Module32Next(hSnapshot, &me32) );
		}
	}

	CloseHandle(hSnapshot);
	return address;
}

uintptr_t mem::Read(HANDLE procHandle, uintptr_t moduleAddr, uintptr_t valueBaseAddr, std::vector<uintptr_t> offset)
{
	uintptr_t offsetNull{ NULL };

	ReadProcessMemory(procHandle, (BYTE*)(moduleAddr + valueBaseAddr), &offsetNull, sizeof(uintptr_t), 0);

	uintptr_t ptrAddr{ offsetNull };

	for ( size_t i = 0; i < offset.size() - 1; i++ )
		ReadProcessMemory(procHandle, (BYTE*)(ptrAddr + offset.at(i)), &ptrAddr, sizeof(uintptr_t), 0);

	return ptrAddr += offset.at(offset.size() - 1);
}

void mem::Write(HWND hWnd)
{

}


