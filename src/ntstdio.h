#ifndef NTSTDIO_H_INCLUDED
#define NTSTDIO_H_INCLUDED



#include "main.h"
#include <winternl.h>

#define SE_CREATE_TOKEN_PRIVILEGE       (2L)
#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE (3L)
#define SE_LOCK_MEMORY_PRIVILEGE        (4L)
#define SE_INCREASE_QUOTA_PRIVILEGE     (5L)
#define SE_UNSOLICITED_INPUT_PRIVILEGE  (6L)
#define SE_MACHINE_ACCOUNT_PRIVILEGE    (7L)
#define SE_TCB_PRIVILEGE                (8L)
#define SE_SECURITY_PRIVILEGE           (9L)
#define SE_TAKE_OWNERSHIP_PRIVILEGE     (10L)
#define SE_LOAD_DRIVER_PRIVILEGE        (10L)
#define SE_SYSTEM_PROFILE_PRIVILEGE     (11L)
#define SE_SYSTEMTIME_PRIVILEGE         (12L)
#define SE_PROF_SINGLE_PROCESS_PRIVILEGE (13L)
#define SE_INC_BASE_PRIORITY_PRIVILEGE  (14L)
#define SE_CREATE_PAGEFILE_PRIVILEGE    (15L)
#define SE_CREATE_PERMANENT_PRIVILEGE   (16L)
#define SE_BACKUP_PRIVILEGE             (17L)
#define SE_RESTORE_PRIVILEGE            (18L)
#define SE_SHUTDOWN_PRIVILEGE           (19L)
#define SE_DEBUG_PRIVILEGE              (20L)
#define SE_AUDIT_PRIVILEGE              (21L)
#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE (22L)
#define SE_CHANGE_NOTIFY_PRIVILEGE      (23L)
#define SE_REMOTE_SHUTDOWN_PRIVILEGE    (24L)
#define KEY_MAKE 0x00 // Key press
#define KEY_BREAK 0x01 // Key release
#define KEY_E0 0x02 // Prefix byte for extended keys
#define KEY_E1 0x04 // Prefix byte for pause/break key


#define RTL_CONSTANT_STRING(s) { sizeof(s) - sizeof((s)[0]), sizeof(s), s }

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;

typedef struct _KEYBOARD_INPUT_DATA {
    USHORT UnitId;
    USHORT MakeCode;
    USHORT Flags;
    USHORT Reserved;
    ULONG  ExtraInformation;
} KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;

typedef enum _EVENT_TYPE
{
    NotificationEvent,
    SynchronizationEvent
} EVENT_TYPE;



typedef VOID (NTAPI *PIO_APC_ROUTINE)(
    _In_ PVOID ApcContext,
    _In_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_ ULONG Reserved
);





typedef struct _KBD_RECORD {
  WORD  wVirtualScanCode;
  DWORD dwControlKeyState;
  UCHAR AsciiChar;
  BOOL  bKeyDown;
} KBD_RECORD, *PKBD_RECORD;



extern NTSTATUS NTAPI NtDeviceIoControlFile(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK, ULONG, PVOID, ULONG, PVOID, ULONG);
extern NTSTATUS NTAPI NtClearEvent(HANDLE);
extern NTSTATUS NTAPI NtDisplayString(PUNICODE_STRING);
extern NTSTATUS NTAPI NtAllocateVirtualMemory(HANDLE, PVOID, ULONG_PTR, PSIZE_T, ULONG, ULONG);
extern NTSTATUS NTAPI NtFreeVirtualMemory(HANDLE, PVOID, PSIZE_T, ULONG);
extern NTSTATUS NTAPI NtDelayExecution(BOOL, PLARGE_INTEGER);
extern NTSTATUS NTAPI RtlAdjustPrivilege(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
extern NTSTATUS NTAPI NtRaiseHardError(NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);
extern NTSTATUS NTAPI IoGetKeyboardState(KEYBOARD_INPUT_DATA*);
extern NTSTATUS NTAPI NtCreateEvent(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, EVENT_TYPE, BOOL);
extern NTSTATUS NTAPI NtCreateFile(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
extern NTSTATUS NTAPI NtOpenFile(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);
extern NTSTATUS NTAPI NtWaitForSingleObject(HANDLE, BOOLEAN, PLARGE_INTEGER);
extern NTSTATUS NTAPI NtReadFile(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK, PVOID, ULONG, PLARGE_INTEGER, PULONG);
extern NTSTATUS NTAPI NtTerminateProcess(HANDLE ProcessHandle, NTSTATUS ExitStatus);
FORCEINLINE VOID RtlInitUnicodeString(
    _Out_ PUNICODE_STRING DestinationString,
    _In_opt_z_ PCWSTR SourceString
);


#define NtCurrentProcess() ((HANDLE)(LONG_PTR)-1)
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#define FILE_OPEN                           0x00000001

NTSTATUS NTAPI NtPuts(WCHAR* String);
WCHAR* NtItoW(int num, WCHAR* str);
void* ZivAllocateMemory(size_t size);
BOOL ZivFreeMemory(void* address, size_t size);
NTSTATUS NtSleep(int seconds);
NTSTATUS NtBlueScreen(NTSTATUS Status);
WCHAR* NtStatusToWString(NTSTATUS status, WCHAR* str);
NTSTATUS NtOpenKeyboard(PHANDLE KeyboardHandle);
void WaitForKeyPress(HANDLE hKeyboard);
VOID ZivPrintNtStatus(IN WCHAR* FunctionName, IN NTSTATUS Status);

#endif