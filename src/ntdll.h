#ifndef NTDLL_H
#define NTDLL_H

#include <Windows.h>
#include <winternl.h>

#define FILE_PIPE_BYTE_STREAM_TYPE 0
#define FILE_PIPE_MESSAGE_TYPE 1
#define FILE_PIPE_BYTE_STREAM_MODE 0
#define FILE_PIPE_MESSAGE_MODE 1
#define FILE_PIPE_QUEUE_OPERATION 0
#define FILE_PIPE_COMPLETE_OPERATION 1

typedef struct _OBJECT_HANDLE_FLAG_INFORMATION {
    BOOLEAN Inherit;
    BOOLEAN ProtectFromClose;
} OBJECT_HANDLE_FLAG_INFORMATION, *POBJECT_HANDLE_FLAG_INFORMATION;

typedef struct _PROC_THREAD_ATTRIBUTE_ENTRY {
    DWORD_PTR Attribute;
    SIZE_T cbSize;
    PVOID lpValue;
} PROC_THREAD_ATTRIBUTE_ENTRY, *LPPROC_THREAD_ATTRIBUTE_ENTRY;

typedef struct _PROC_THREAD_ATTRIBUTE_LIST {
    DWORD dwFlags;
    ULONG Size;
    ULONG Count;
    ULONG Reserved;
    PULONG Unknown;
    PROC_THREAD_ATTRIBUTE_ENTRY Entries[ANYSIZE_ARRAY];
} PROC_THREAD_ATTRIBUTE_LIST, *LPPROC_THREAD_ATTRIBUTE_LIST;

NTSTATUS NtCreateNamedPipeFile(
    PHANDLE NamedPipeFileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    ULONG WriteModeMessage,
    ULONG ReadModeMessage,
    ULONG NonBlocking,
    ULONG MaxInstances,
    ULONG InBufferSize,
    ULONG OutBufferSize,
    PLARGE_INTEGER DefaultTimeOut);

NTSTATUS NtDuplicateObject(
    HANDLE SourceProcessHandle,
    HANDLE SourceHandle,
    HANDLE TargetProcessHandle,
    PHANDLE TargetHandle,
    ACCESS_MASK DesiredAccess,
    ULONG HandleAttributes,
    ULONG Options);

NTSTATUS NtSetInformationObject(
    HANDLE ObjectHandle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG Length);

NTSTATUS NtTerminateProcess(
    HANDLE ProcessHandle,
    NTSTATUS ExitStatus);

PCWSTR RtlGetNtSystemRoot(
    void);

void RtlSetLastWin32Error(
    ULONG LastError);

#endif //NTDLL_H
