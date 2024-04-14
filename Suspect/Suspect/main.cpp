#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include<stdlib.h>

char s[4] = "[+]";
char e[4] = "[-]";

int error(const char message[]) {
	printf("%s %s: %ld", e, message, GetLastError());
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
	if (argc != 3) error("Usage: program.exe [PID] [dllPath]");

	DWORD pid, tid = NULL;
	HANDLE hProcess, hThread = NULL;
	LPVOID buffer = NULL;
	char dllPath[100];

	strcpy_s(dllPath, argv[2]);

	pid = atoi(argv[1]);
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	if (hProcess == NULL) error("Failed to open handle to process.");
	printf("%s Got Handle to process (%ld)\n", s, pid);

	buffer = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (buffer == NULL) {
		CloseHandle(hProcess);
		error("Failed to reserve memory in target process!");
	}

	if (WriteProcessMemory(hProcess, buffer, dllPath, sizeof(dllPath) + 1, NULL) == 0) {
		CloseHandle(hProcess);
		error("Failed to write process memory!");
	}

	hThread = CreateRemoteThreadEx(hProcess, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryA), buffer, 0, NULL, &tid);
	if (hThread == NULL) {
		CloseHandle(hProcess);
		error("Failed to open handle to remote thread.");
	}

	WaitForSingleObject(hThread, INFINITE);
	printf("%s Successfully executed thread code. Exiting...", s);
	CloseHandle(hProcess);
	CloseHandle(hThread);

	return 0;
}

