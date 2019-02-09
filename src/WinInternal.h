#ifndef WININTERNAL_H
#define WININTERNAL_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

typedef long NTSTATUS;

// From DetoursNT/DetoursNT.h
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((Status) >= 0)
#endif
#define NtCurrentProcess() ((HANDLE)(LONG_PTR)-1)
#define NtCurrentThread() ((HANDLE)(LONG_PTR)-2)

// Time processhacker/phlib/include/phsup.h
#define TICKS_PER_NS ((long long)1 * 10)
#define TICKS_PER_MS (TICKS_PER_NS * 1000)
#define TICKS_PER_SEC (TICKS_PER_MS * 1000)
#define TICKS_PER_MIN (TICKS_PER_SEC * 60)

// Flags for NtCreateNamedPipeFile
#define FILE_PIPE_BYTE_STREAM_TYPE 0
#define FILE_PIPE_MESSAGE_TYPE 1
#define FILE_PIPE_BYTE_STREAM_MODE 0
#define FILE_PIPE_MESSAGE_MODE 1
#define FILE_PIPE_QUEUE_OPERATION 0
#define FILE_PIPE_COMPLETE_OPERATION 1

// Flags from winternl.h
#define OBJ_INHERIT 2
#define OBJ_CASE_INSENSITIVE 64

#ifndef __MINGW32__
#define FILE_CREATE 2
#define FILE_SYNCHRONOUS_IO_NONALERT 32
#define FILE_NON_DIRECTORY_FILE 64
#endif

// Some handmade
#define ToULong(x) (unsigned long)(unsigned long long)(x)
#define ToHandle(x) (void*)(unsigned long long)(x)

typedef enum _OBJECT_INFORMATION_CLASS {
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectTypesInformation,
    ObjectHandleFlagInformation,
    ObjectSessionInformation,
    ObjectSessionObjectInformation,
    MaxObjectInfoClass
} OBJECT_INFORMATION_CLASS;

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

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, ANSI_STRING, *PSTRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _CURDIR {
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR, *PCURDIR;

typedef struct _RTL_DRIVE_LETTER_CURDIR {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

#define RTL_MAX_DRIVE_LETTERS 32

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    ULONG MaximumLength;
    ULONG Length;
    ULONG Flags;
    ULONG DebugFlags;
    HANDLE ConsoleHandle;
    ULONG ConsoleFlags;
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;
    CURDIR CurrentDirectory;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    PWSTR Environment;
    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopInfo;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    RTL_DRIVE_LETTER_CURDIR CurrentDirectores[RTL_MAX_DRIVE_LETTERS];
    SIZE_T EnvironmentSize;
    SIZE_T EnvironmentVersion;
    PVOID PackageDependencyData;
    ULONG ProcessGroupId;
    ULONG LoaderThreads;
    UNICODE_STRING RedirectionDllName;
    UNICODE_STRING HeapPartitionName;
    PVOID DefaultThreadpoolCpuSetMasks;
    ULONG DefaultThreadpoolCpuSetMaskCount;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB {
    BYTE Unused[28];
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID SubSystemData;
    HANDLE ProcessHeap;
} PEB, *PPEB;

typedef struct _TEB {
    PVOID Unused[12];
    PPEB ProcessEnvironmentBlock;
    ULONG LastErrorValue;
} TEB, *PTEB;

// macros with TEB
#define NtCurrentPeb() \
NtCurrentTeb()->ProcessEnvironmentBlock

#define UserProcessParameter() \
NtCurrentTeb()->ProcessEnvironmentBlock->ProcessParameters

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    } u;
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

NTSTATUS
NTAPI
NtClose(HANDLE FileHandle);

NTSTATUS
NTAPI
NtCreateNamedPipeFile(PHANDLE NamedPipeFileHandle,
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

NTSTATUS
NTAPI
NtDuplicateObject(HANDLE SourceProcessHandle,
                  HANDLE SourceHandle,
                  HANDLE TargetProcessHandle,
                  PHANDLE TargetHandle,
                  ACCESS_MASK DesiredAccess,
                  ULONG HandleAttributes,
                  ULONG Options);

NTSTATUS
NTAPI
NtOpenFile(PHANDLE FileHandle,
           ACCESS_MASK DesiredAccess,
           POBJECT_ATTRIBUTES ObjectAttributes,
           PIO_STATUS_BLOCK IoStatusBlock,
           ULONG ShareAccess,
           ULONG OpenOptions);

NTSTATUS
NTAPI
NtQueryObject(HANDLE FileHandle,
              OBJECT_INFORMATION_CLASS ObjectInformationClass,
              PVOID ObjectInformation,
              ULONG ObjectInformationLength,
              PULONG ReturnLength);

NTSTATUS
NTAPI
NtSetInformationObject(HANDLE ObjectHandle,
                       OBJECT_INFORMATION_CLASS ObjectInformationClass,
                       PVOID ObjectInformation,
                       ULONG Length);

NTSTATUS
NTAPI
NtTerminateProcess(HANDLE ProcessHandle,
                   NTSTATUS ExitStatus);

NTSTATUS
NTAPI
NtWriteFile(HANDLE FileHandle,
            HANDLE Event,
            PVOID ApcRoutine,
            PVOID ApcContext,
            PIO_STATUS_BLOCK IoStatusBlock,
            PVOID Buffer,
            ULONG Length,
            PLARGE_INTEGER ByteOffset,
            PULONG Key);

PVOID
NTAPI
RtlAllocateHeap(PVOID HeapHandle,
                ULONG Flags,
                SIZE_T Size);

BOOLEAN
NTAPI
RtlFreeHeap(PVOID HeapHandle,
            ULONG Flags,
            PVOID BaseAddress);

PCWSTR
NTAPI
RtlGetNtSystemRoot(void);

ULONG
NTAPI
RtlNtStatusToDosError(NTSTATUS Status);

void
NTAPI
RtlSetLastWin32Error(ULONG LastError);

#endif //WININTERNAL_H
