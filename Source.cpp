#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#pragma warning( disable : 6387; disable : 6333 )

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

int main()
{
	Sleep(10000);
	// DLL path
	LPCSTR DllPath = "D:\\Visual Studio Projects\\C++\\DLLInjector\\Release\\DynamicLinkLibrary.dll";
	LPCSTR WName = "League of Legends (TM) Client";

	/*
	uintptr_t base = (uintptr_t)GetModuleHandle(NULL); // Get base address of current process.

	std::cout << "Start address: " << base << std::endl;
	std::cout << "Current process ID " << GetCurrentProcessId() << std::endl;
	std::cout << GetModuleBaseAddress(GetCurrentProcessId(), L"DLLInjector.exe") << std::endl;
	std::cin.get();*/

	HWND hwnd = FindWindowA(NULL, WName);

	if (hwnd == NULL)
	{
		std::cout << "[ERROR] Could not find any window with name: " << WName;
		return 0;
	}

	DWORD procID;
	GetWindowThreadProcessId(hwnd, &procID);


	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, procID);

	if (hProcess == NULL)
	{
		std::cout << "[ERROR] Could not open process with ID " << procID << std::endl;
		return 0;
	}

	std::cout << "Found process " << procID << std::endl;

	// Allocate memory for the dllpath inside the target process' memory
	LPVOID pDllpath = VirtualAllocEx(hProcess, 0, strlen(DllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

	if (pDllpath == NULL)
	{
		std::cout << "[ERROR] Could not allocate memory for the DLL " << std::endl;
		return 0;
	}

	// Write the path to the address memory
	WriteProcessMemory(hProcess, pDllpath, (LPVOID)DllPath, strlen(DllPath) + 1, 0);

	FARPROC loadProcess = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA");

	if (loadProcess == NULL)
	{
		std::cout << "[ERROR] Could not load function LoadLibraryA from Kernel32.dll " << std::endl;
		return 0;
	}

	HANDLE hLoadThread = CreateRemoteThread(hProcess, 0, 0,
		(LPTHREAD_START_ROUTINE)loadProcess, pDllpath, 0, 0);


	if (hLoadThread == NULL)
	{
		std::cout << "[ERROR] Could not create Remote Thread " << std::endl;
		return 0;
	}


	WaitForSingleObject(hLoadThread, INFINITE);

	std::cout << "Dll path allocated at: " << pDllpath << std::endl;

	if (!VirtualFreeEx(hProcess, pDllpath, 0, MEM_RELEASE))
	{
		std::cout << "Failed to release the allocated memory for the DLL path" << std::endl;
	}
	return 0;
}