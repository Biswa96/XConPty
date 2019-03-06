#include "WinInternal.h"
#include <stdio.h>

void
WINAPI
LogResult(HRESULT hResult, PWSTR Function)
{
    if(hResult < 0)
        wprintf(L"[-] ERROR %ld %ls\n", (hResult & 0xFFFF), Function);
}

void
WINAPI
LogStatus(NTSTATUS Status, PWSTR Function)
{
    if(Status < 0)
        wprintf(L"[-] NTSTATUS 0x%08lX %ls\n", Status, Function);
}

ULONG
WINAPI
BaseSetLastNTError(NTSTATUS Status)
{
    ULONG Result;

    Result = RtlNtStatusToDosError(Status);
    RtlSetLastWin32Error(Result);
    return Result;
}

BOOL
WINAPI
X_InitializeProcThreadAttributeList(LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
                                    DWORD dwAttributeCount,
                                    DWORD dwFlags,
                                    PSIZE_T lpSize)
{
    if (dwFlags) {
        // STATUS_INVALID_PARAMETER_3
    }
    if (dwAttributeCount > sizeof(PROC_THREAD_ATTRIBUTE_ENTRY)) {
        // STATUS_INVALID_PARAMETER_2
    }

    BOOL Result = TRUE;
    ULONG Size = (ULONG)sizeof(PROC_THREAD_ATTRIBUTE_ENTRY) * (dwAttributeCount + 1);
    if (lpAttributeList && *lpSize >= Size)
    {
        lpAttributeList->dwFlags = 0;
        lpAttributeList->Unknown = NULL;
        lpAttributeList->Size = dwAttributeCount;
        lpAttributeList->Count = 0;
    }
    else
    {
        Result = FALSE;
        RtlSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
    }

    // Return size if buffer size does not match
    *lpSize = Size;
    return Result;
}

BOOL
WINAPI
X_UpdateProcThreadAttribute(LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
                            DWORD dwFlags,
                            DWORD_PTR Attribute,
                            PVOID lpValue,
                            SIZE_T cbSize,
                            PVOID lpPreviousValue,
                            PSIZE_T lpReturnSize)
{
    DWORD Flags = 1 << Attribute;

    if (!(Attribute & PROC_THREAD_ATTRIBUTE_INPUT)) {
        if (lpAttributeList->Count == lpAttributeList->Size) {
            // STATUS_UNSUCCESSFUL
        }
        if (Flags & lpAttributeList->dwFlags) {
            // STATUS_OBJECT_NAME_EXISTS
        }
        if (lpPreviousValue) {
            // STATUS_INVALID_PARAMETER_6
        }
        if (dwFlags & 1) {
            // STATUS_INVALID_PARAMETER_2
        }
    }

    if (Attribute & (PROC_THREAD_ATTRIBUTE_THREAD | PROC_THREAD_ATTRIBUTE_INPUT)
        && lpReturnSize) {
        // STATUS_INVALID_PARAMETER_7
    }

    if (Flags)
    {
        ++lpAttributeList->Count;
        lpAttributeList->dwFlags |= Flags;
        lpAttributeList->Entries[0].Attribute = Attribute;
        lpAttributeList->Entries[0].cbSize = cbSize;
        lpAttributeList->Entries[0].lpValue = lpValue;
        // lpAttributeList->Size previously set
        // lpAttributeList->Unknown unchanged
    }

    return TRUE;
}

BOOL
WINAPI
X_CreatePipe(PHANDLE hReadPipe,
             PHANDLE hWritePipe,
             LPSECURITY_ATTRIBUTES lpPipeAttributes,
             DWORD nSize)
{
    NTSTATUS Status;
    UNICODE_STRING PipeObject;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;

    PVOID lpSecurityDescriptor = NULL;
    ULONG Attributes = OBJ_CASE_INSENSITIVE; // Always case insensitive
    HANDLE DeviceHandle = NULL, ReadPipeHandle = NULL, WritePipeHandle = NULL;
    LARGE_INTEGER DefaultTimeOut;
    DefaultTimeOut.QuadPart = -2 * TICKS_PER_MIN;
    DWORD Size = 0x1000;
    if (nSize)
        Size = nSize;

    // Open NamedPipe device
    RtlZeroMemory(&PipeObject, sizeof PipeObject);
    RtlInitUnicodeString(&PipeObject, L"\\Device\\NamedPipe\\");

    RtlZeroMemory(&ObjectAttributes, sizeof ObjectAttributes);
    ObjectAttributes.Length = sizeof ObjectAttributes;
    ObjectAttributes.ObjectName = &PipeObject;

    Status = NtOpenFile(&DeviceHandle,
                        GENERIC_READ | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(Status))
    {
        LogStatus(Status, L"NtOpenFile");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    // Set security Attributes
    if (lpPipeAttributes)
    {
        lpSecurityDescriptor = lpPipeAttributes->lpSecurityDescriptor;
        if (lpPipeAttributes->bInheritHandle)
            Attributes |= OBJ_INHERIT;
    }

    // Create Named Pipe
    RtlZeroMemory(&PipeObject, sizeof PipeObject);
    ObjectAttributes.RootDirectory = DeviceHandle;
    ObjectAttributes.Length = sizeof ObjectAttributes;
    ObjectAttributes.ObjectName = &PipeObject;
    ObjectAttributes.Attributes = Attributes;
    ObjectAttributes.SecurityDescriptor = lpSecurityDescriptor;
    ObjectAttributes.SecurityQualityOfService = NULL;

    Status = NtCreateNamedPipeFile(&ReadPipeHandle,
                                   GENERIC_READ | SYNCHRONIZE | FILE_WRITE_ATTRIBUTES,
                                   &ObjectAttributes,
                                   &IoStatusBlock,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   FILE_CREATE,
                                   FILE_SYNCHRONOUS_IO_NONALERT,
                                   FILE_PIPE_BYTE_STREAM_TYPE,
                                   FILE_PIPE_BYTE_STREAM_MODE,
                                   FILE_PIPE_QUEUE_OPERATION,
                                   1,
                                   Size,
                                   Size,
                                   &DefaultTimeOut);

    if (!NT_SUCCESS(Status))
    {
        LogStatus(Status, L"NtCreateNamedPipeFile");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    // Open write handle for the pipe
    ObjectAttributes.RootDirectory = ReadPipeHandle;
    ObjectAttributes.ObjectName = &PipeObject;
    ObjectAttributes.Length = sizeof ObjectAttributes;
    ObjectAttributes.Attributes = Attributes;
    ObjectAttributes.SecurityDescriptor = lpSecurityDescriptor;
    ObjectAttributes.SecurityQualityOfService = NULL;

    Status = NtOpenFile(&WritePipeHandle,
                        GENERIC_WRITE | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);

    if (!NT_SUCCESS(Status))
    {
        LogStatus(Status, L"NtOpenFile");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    // Return pipe handles
    *hReadPipe = ReadPipeHandle;
    *hWritePipe = WritePipeHandle;
    return TRUE;
}

NTSTATUS
WINAPI
X_CreateHandle(PHANDLE FileHandle,
               PWSTR Buffer,
               ACCESS_MASK DesiredAccess,
               HANDLE RootDirectory,
               BOOLEAN IsInheritable,
               ULONG OpenOptions)
{
    NTSTATUS Status;
    UNICODE_STRING ObjectName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG Attributes = OBJ_CASE_INSENSITIVE;
    if (IsInheritable)
        Attributes |= OBJ_INHERIT;

    RtlZeroMemory(&ObjectName, sizeof ObjectName);
    RtlInitUnicodeString(&ObjectName, Buffer);

    RtlZeroMemory(&ObjectAttributes, sizeof ObjectAttributes);
    ObjectAttributes.RootDirectory = RootDirectory;
    ObjectAttributes.Length = sizeof ObjectAttributes;
    ObjectAttributes.Attributes = Attributes;
    ObjectAttributes.ObjectName = &ObjectName;

    Status = NtOpenFile(FileHandle,
                      DesiredAccess,
                      &ObjectAttributes,
                      &IoStatusBlock,
                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                      OpenOptions);
    return Status;
}

BOOL
WINAPI
X_DuplicateHandle(HANDLE hSourceProcessHandle,
                  HANDLE hSourceHandle,
                  HANDLE hTargetProcessHandle,
                  LPHANDLE lpTargetHandle,
                  DWORD dwDesiredAccess,
                  BOOL bInheritHandle,
                  DWORD dwOptions)
{
    NTSTATUS Status;

    switch (ToULong(hSourceHandle))
    {
    case STD_ERROR_HANDLE:
        hSourceHandle = UserProcessParameter()->StandardError;
        break;

    case STD_OUTPUT_HANDLE:
        hSourceHandle = UserProcessParameter()->StandardOutput;
        break;

    case STD_INPUT_HANDLE:
        hSourceHandle = UserProcessParameter()->StandardInput;
        break;
    }

    Status = NtDuplicateObject(hSourceProcessHandle,
                               hSourceHandle,
                               hTargetProcessHandle,
                               lpTargetHandle,
                               dwDesiredAccess,
                               bInheritHandle != FALSE ? OBJ_INHERIT : 0,
                               dwOptions);

    if (!NT_SUCCESS(Status))
    {
        LogStatus(Status, L"NtDuplicateObject");
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}

HANDLE
WINAPI
X_GetStdHandle(DWORD nStdHandle)
{
    HANDLE hSourceHandle;

    switch (nStdHandle)
    {
    case STD_ERROR_HANDLE:
        hSourceHandle = UserProcessParameter()->StandardError;
        break;

    case STD_OUTPUT_HANDLE:
        hSourceHandle = UserProcessParameter()->StandardOutput;
        break;

    case STD_INPUT_HANDLE:
        if (UserProcessParameter()->WindowFlags & STARTF_USEHOTKEY)
            return NULL;
        hSourceHandle = UserProcessParameter()->StandardInput;
        break;

    default:
        hSourceHandle = (HANDLE)-1;
    }

    if (hSourceHandle == (HANDLE)-1)
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
    return hSourceHandle;
}

BOOL
WINAPI
X_SetHandleInformation(HANDLE hObject,
                       DWORD dwMask,
                       DWORD dwFlags)
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION ObjectInformation = { 0 };

    switch (ToULong(hObject))
    {
    case STD_ERROR_HANDLE:
        hObject = UserProcessParameter()->StandardError;
        break;

    case STD_OUTPUT_HANDLE:
        hObject = UserProcessParameter()->StandardOutput;
        break;

    case STD_INPUT_HANDLE:
        hObject = UserProcessParameter()->StandardInput;
        break;
    }

    Status = NtQueryObject(hObject,
                           ObjectHandleFlagInformation,
                           &ObjectInformation,
                           sizeof ObjectInformation,
                           NULL);

    if (!NT_SUCCESS(Status))
    {
        LogStatus(Status, L"NtQueryObject");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (dwMask & HANDLE_FLAG_INHERIT)
        ObjectInformation.Inherit = dwFlags & HANDLE_FLAG_INHERIT;

    if (dwMask & HANDLE_FLAG_PROTECT_FROM_CLOSE)
        ObjectInformation.ProtectFromClose = (dwFlags >> 1) & HANDLE_FLAG_INHERIT;

    Status = NtSetInformationObject(hObject,
                                    ObjectHandleFlagInformation,
                                    &ObjectInformation,
                                    sizeof ObjectInformation);

    if (!NT_SUCCESS(Status))
    {
        LogStatus(Status, L"NtSetInformationObject");
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
WINAPI
X_TerminateProcess(HANDLE hProcess,
                   UINT uExitCode)
{
    if (!hProcess)
    {
        RtlSetLastWin32Error(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    // RtlReportSilentProcessExit(); goes to SendMessageToWERService();
    // which opens WerFault.exe aka. User Data Collector

    NTSTATUS Status;
    Status = NtTerminateProcess(hProcess, uExitCode);

    if (!NT_SUCCESS(Status))
    {
        LogStatus(Status, L"NtTerminateProcess");
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}
