#ifndef KERNELBASE_H
#define KERNELBASE_H

#include <Windows.h>

void Log(
    ULONG Result,
    PWSTR Function);

HANDLE X_GetCurrentProcess(
    void);

DWORD X_GetLastError(
    void);

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
    PSECURITY_ATTRIBUTES lpPipeAttributes,
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

HANDLE X_GetStdHandle(
    DWORD nStdHandle);

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
} X_HPCON;

HRESULT X_CreatePseudoConsole(
    COORD size,
    HANDLE hInput,
    HANDLE hOutput,
    DWORD dwFlags,
    X_HPCON* hpCon);

void X_ClosePseudoConsole(
    X_HPCON hpCon);

#endif //KERNELBASE_H
