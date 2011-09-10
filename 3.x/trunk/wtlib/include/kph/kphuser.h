#ifndef _PH_KPHUSER_H
#define _PH_KPHUSER_H

#include <kphapi.h>

typedef struct _KPH_PARAMETERS
{
    KPH_SECURITY_LEVEL SecurityLevel;
} KPH_PARAMETERS, *PKPH_PARAMETERS;

LCLIBAPI
NTSTATUS
NTAPI
KphConnect(
    __in_opt PWSTR DeviceName
    );

LCLIBAPI
NTSTATUS
NTAPI
KphConnect2(
    __in_opt PWSTR DeviceName,
    __in PWSTR FileName
    );

LCLIBAPI
NTSTATUS
NTAPI
KphConnect2Ex(
    __in_opt PWSTR DeviceName,
    __in PWSTR FileName,
    __in_opt PKPH_PARAMETERS Parameters
    );

LCLIBAPI
NTSTATUS
NTAPI
KphDisconnect(
    VOID
    );

LCLIBAPI
BOOLEAN
NTAPI
KphIsConnected(
    VOID
    );

LCLIBAPI
NTSTATUS
NTAPI
KphSetParameters(
    __in_opt PWSTR DeviceName,
    __in PKPH_PARAMETERS Parameters
    );

LCLIBAPI
NTSTATUS
NTAPI
KphInstall(
    __in_opt PWSTR DeviceName,
    __in PWSTR FileName
    );

LCLIBAPI
NTSTATUS
NTAPI
KphInstallEx(
    __in_opt PWSTR DeviceName,
    __in PWSTR FileName,
    __in_opt PKPH_PARAMETERS Parameters
    );

LCLIBAPI
NTSTATUS
NTAPI
KphUninstall(
    __in_opt PWSTR DeviceName
    );

LCLIBAPI
NTSTATUS
NTAPI
KphGetFeatures(
    __out PULONG Features
    );

LCLIBAPI
NTSTATUS
NTAPI
KphOpenProcess(
    __out PHANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in PCLIENT_ID ClientId
    );

LCLIBAPI
NTSTATUS
NTAPI
KphOpenProcessToken(
    __in HANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __out PHANDLE TokenHandle
    );

LCLIBAPI
NTSTATUS
NTAPI
KphOpenProcessJob(
    __in HANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __out PHANDLE JobHandle
    );

LCLIBAPI
NTSTATUS
NTAPI
KphSuspendProcess(
    __in HANDLE ProcessHandle
    );

LCLIBAPI
NTSTATUS
NTAPI
KphResumeProcess(
    __in HANDLE ProcessHandle
    );

LCLIBAPI
NTSTATUS
NTAPI
KphTerminateProcess(
    __in HANDLE ProcessHandle,
    __in NTSTATUS ExitStatus
    );

LCLIBAPI
NTSTATUS
NTAPI
KphReadVirtualMemory(
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __out_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesRead
    );

LCLIBAPI
NTSTATUS
NTAPI
KphWriteVirtualMemory(
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress,
    __in_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesWritten
    );

LCLIBAPI
NTSTATUS
NTAPI
KphReadVirtualMemoryUnsafe(
    __in_opt HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __out_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesRead
    );

LCLIBAPI
NTSTATUS
NTAPI
KphQueryInformationProcess(
    __in HANDLE ProcessHandle,
    __in KPH_PROCESS_INFORMATION_CLASS ProcessInformationClass,
    __out_bcount(ProcessInformationLength) PVOID ProcessInformation,
    __in ULONG ProcessInformationLength,
    __out_opt PULONG ReturnLength
    );

LCLIBAPI
NTSTATUS
NTAPI
KphSetInformationProcess(
    __in HANDLE ProcessHandle,
    __in KPH_PROCESS_INFORMATION_CLASS ProcessInformationClass,
    __in_bcount(ProcessInformationLength) PVOID ProcessInformation,
    __in ULONG ProcessInformationLength
    );

LCLIBAPI
NTSTATUS
NTAPI
KphOpenThread(
    __out PHANDLE ThreadHandle,
    __in ACCESS_MASK DesiredAccess,
    __in PCLIENT_ID ClientId
    );

LCLIBAPI
NTSTATUS
NTAPI
KphOpenThreadProcess(
    __in HANDLE ThreadHandle,
    __in ACCESS_MASK DesiredAccess,
    __out PHANDLE ProcessHandle
    );

LCLIBAPI
NTSTATUS
NTAPI
KphTerminateThread(
    __in HANDLE ThreadHandle,
    __in NTSTATUS ExitStatus
    );

LCLIBAPI
NTSTATUS
NTAPI
KphTerminateThreadUnsafe(
    __in HANDLE ThreadHandle,
    __in NTSTATUS ExitStatus
    );

LCLIBAPI
NTSTATUS
NTAPI
KphGetContextThread(
    __in HANDLE ThreadHandle,
    __inout PCONTEXT ThreadContext
    );

LCLIBAPI
NTSTATUS
NTAPI
KphSetContextThread(
    __in HANDLE ThreadHandle,
    __in PCONTEXT ThreadContext
    );

LCLIBAPI
NTSTATUS
NTAPI
KphCaptureStackBackTraceThread(
    __in HANDLE ThreadHandle,
    __in ULONG FramesToSkip,
    __in ULONG FramesToCapture,
    __out_ecount(FramesToCapture) PVOID *BackTrace,
    __out_opt PULONG CapturedFrames,
    __out_opt PULONG BackTraceHash
    );

LCLIBAPI
NTSTATUS
NTAPI
KphQueryInformationThread(
    __in HANDLE ThreadHandle,
    __in KPH_THREAD_INFORMATION_CLASS ThreadInformationClass,
    __out_bcount(ProcessInformationLength) PVOID ThreadInformation,
    __in ULONG ThreadInformationLength,
    __out_opt PULONG ReturnLength
    );

LCLIBAPI
NTSTATUS
NTAPI
KphSetInformationThread(
    __in HANDLE ThreadHandle,
    __in KPH_THREAD_INFORMATION_CLASS ThreadInformationClass,
    __in_bcount(ThreadInformationLength) PVOID ThreadInformation,
    __in ULONG ThreadInformationLength
    );

LCLIBAPI
NTSTATUS
NTAPI
KphEnumerateProcessHandles(
    __in HANDLE ProcessHandle,
    __out_bcount(BufferLength) PVOID Buffer,
    __in_opt ULONG BufferLength,
    __out_opt PULONG ReturnLength
    );

LCLIBAPI
NTSTATUS
NTAPI
KphQueryInformationObject(
    __in HANDLE ProcessHandle,
    __in HANDLE Handle,
    __in KPH_OBJECT_INFORMATION_CLASS ObjectInformationClass,
    __out_bcount(ObjectInformationLength) PVOID ObjectInformation,
    __in ULONG ObjectInformationLength,
    __out_opt PULONG ReturnLength
    );

LCLIBAPI
NTSTATUS
NTAPI
KphSetInformationObject(
    __in HANDLE ProcessHandle,
    __in HANDLE Handle,
    __in KPH_OBJECT_INFORMATION_CLASS ObjectInformationClass,
    __in_bcount(ObjectInformationLength) PVOID ObjectInformation,
    __in ULONG ObjectInformationLength
    );

LCLIBAPI
NTSTATUS
NTAPI
KphDuplicateObject(
    __in HANDLE SourceProcessHandle,
    __in HANDLE SourceHandle,
    __in_opt HANDLE TargetProcessHandle,
    __out_opt PHANDLE TargetHandle,
    __in ACCESS_MASK DesiredAccess,
    __in ULONG HandleAttributes,
    __in ULONG Options
    );

LCLIBAPI
NTSTATUS
NTAPI
KphOpenDriver(
    __out PHANDLE DriverHandle,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

LCLIBAPI
NTSTATUS
NTAPI
KphQueryInformationDriver(
    __in HANDLE DriverHandle,
    __in DRIVER_INFORMATION_CLASS DriverInformationClass,
    __out_bcount(DriverInformationLength) PVOID DriverInformation,
    __in ULONG DriverInformationLength,
    __out_opt PULONG ReturnLength
    );

#endif
