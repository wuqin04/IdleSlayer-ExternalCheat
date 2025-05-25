#pragma once
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <vector>

namespace mem
{
	// get the window handler 
	void GetWinHandle(HWND& hWndBuffer);

	// get process id by using GetWindowThreadProcessId
	DWORD GetProcID(HWND hWnd);

	// get handle count 
	DWORD GetHandleCount(HANDLE hProcess);

	// get thread count
	DWORD GetThreadCount(DWORD processID);

	// get module address by using CreateToolhelp32Snapshot
	uintptr_t GetModuleAddress(DWORD ProcID, const TCHAR* moduleName);

	// read the memory process of the address
	uintptr_t Read(HANDLE procHandle, uintptr_t moduleAddr, uintptr_t valueBaseAddr, std::vector<uintptr_t> offset);

	// write the memory process of the address
	void Write(HWND hWnd);
}