#ifndef KERNELBASE_H
#define KERNELBASE_H

void
WINAPI
LogResult(HRESULT hResult, PWSTR Function);

void
WINAPI
LogStatus(NTSTATUS Status, PWSTR Function);

ULONG
WINAPI
BaseSetLastNTError(NTSTATUS Status);

BOOL
WINAPI
X_InitializeProcThreadAttributeList(LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
                                    DWORD dwAttributeCount,
                                    DWORD dwFlags,
                                    PSIZE_T lpSize);

BOOL
WINAPI
X_UpdateProcThreadAttribute(LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
                            DWORD dwFlags,
                            DWORD_PTR Attribute,
                            PVOID lpValue,
                            SIZE_T cbSize,
                            PVOID lpPreviousValue,
                            PSIZE_T lpReturnSize);

BOOL
WINAPI
X_CreatePipe(PHANDLE hReadPipe,
             PHANDLE hWritePipe,
             LPSECURITY_ATTRIBUTES lpPipeAttributes,
             DWORD nSize);

NTSTATUS
WINAPI
X_CreateHandle(PHANDLE FileHandle,
               PWSTR Buffer,
               ACCESS_MASK DesiredAccess,
               HANDLE RootDirectory,
               BOOLEAN IsInheritable,
               ULONG OpenOptions);

BOOL
WINAPI
X_DuplicateHandle(HANDLE hSourceProcessHandle,
                  HANDLE hSourceHandle,
                  HANDLE hTargetProcessHandle,
                  LPHANDLE lpTargetHandle,
                  DWORD dwDesiredAccess,
                  BOOL bInheritHandle,
                  DWORD dwOptions);

HANDLE
WINAPI
X_GetStdHandle(DWORD nStdHandle);

BOOL
WINAPI
X_SetHandleInformation(HANDLE hObject,
                       DWORD dwMask,
                       DWORD dwFlags);

BOOL
WINAPI
X_TerminateProcess(HANDLE hProcess,
                   UINT uExitCode);

#endif //KERNELBASE_H
