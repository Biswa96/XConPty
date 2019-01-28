#ifndef KERNELBASE_H
#define KERNELBASE_H

#include <Windows.h>

void Log(ULONG Result, PWSTR Function);

DWORD X_GetLastError(void);

HANDLE X_GetProcessHeap(void);

ULONG BaseSetLastNTError(
    NTSTATUS Status);

BOOL X_InitializeProcThreadAttributeList(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD dwAttributeCount,
    DWORD dwFlags,
    PSIZE_T lpSize);

BOOL X_UpdateProcThreadAttribute(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD dwFlags,
    DWORD_PTR Attribute,
    PVOID lpValue,
    SIZE_T cbSize,
    PVOID lpPreviousValue,
    PSIZE_T lpReturnSize);

BOOL X_CreatePipe(
    PHANDLE hReadPipe,
    PHANDLE hWritePipe,
    LPSECURITY_ATTRIBUTES lpPipeAttributes,
    DWORD nSize);

NTSTATUS X_CreateHandle(
    PHANDLE FileHandle,
    PWSTR Buffer,
    ACCESS_MASK DesiredAccess,
    HANDLE RootDirectory,
    BOOLEAN IsInheritable,
    ULONG OpenOptions);

BOOL X_DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions);

HANDLE X_GetStdHandle(DWORD nStdHandle);

BOOL X_SetHandleInformation(
    HANDLE hObject,
    DWORD dwMask,
    DWORD dwFlags);

BOOL X_TerminateProcess(
    HANDLE hProcess,
    UINT uExitCode);

typedef struct _X_HPCON {
    HANDLE hWritePipe;
    HANDLE hConDrvReference;
    HANDLE hConHostProcess;
} X_HPCON, *PX_HPCON;

HRESULT X_CreatePseudoConsole(
    COORD ConsoleSize,
    HANDLE hInput,
    HANDLE hOutput,
    DWORD dwFlags,
    PX_HPCON hpCon);

#define RESIZE_CONHOST_SIGNAL_BUFFER 8

typedef struct _RESIZE_PSEUDO_CONSOLE_BUFFER {
    short Flags;
    short SizeX;
    short SizeY;
} RESIZE_PSEUDO_CONSOLE_BUFFER, *PRESIZE_PSEUDO_CONSOLE_BUFFER;

HRESULT X_ResizePseudoConsole(PX_HPCON hPC, COORD size);
void X_ClosePseudoConsole(PX_HPCON hpCon);

#endif //KERNELBASE_H
