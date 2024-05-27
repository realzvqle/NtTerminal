#include "ntstdio.h"
#include <minwindef.h>
#include <winnt.h>
#include <winternl.h>








NTSTATUS NtPuts(WCHAR* String){
    UNICODE_STRING uString;
    PCWSTR text = String;
    RtlInitUnicodeString(&uString, text);
    return NtDisplayString(&uString); 
}

VOID ZivPrintNtStatus(IN WCHAR* FunctionName, IN NTSTATUS Status){
    WCHAR* String = ZivAllocateMemory(2 * sizeof(WCHAR));
    if(!String){
        NtPuts(L"Memory Allocation Failure");
        return;
    }
    NtStatusToWString(Status, String);
    NtPuts(L"\n");
    NtPuts(FunctionName);
    NtPuts(L" Failed With NTSTATUS ");
    NtPuts(String);
    NtPuts(L"\n");
    ZivFreeMemory(String, 2*sizeof(WCHAR));
}

UNICODE_STRING RtlConstentString(WCHAR* String){
    UNICODE_STRING uString;
    PCWSTR text = String;
    RtlInitUnicodeString(&uString, text);
    return uString;
}   


NTSTATUS NtBlueScreen(NTSTATUS Status){
    BOOLEAN bl;
    unsigned long response;
    NTSTATUS privlegestat = RtlAdjustPrivilege(19, TRUE, FALSE, &bl);
    if(!NT_SUCCESS(privlegestat)){
        return privlegestat;
    }
    return NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, 0, 6, &response); 
}



void* ZivAllocateMemory(size_t size) {
    void* baseAddress = NULL;
    NTSTATUS status = NtAllocateVirtualMemory(
        NtCurrentProcess(),  
        &baseAddress,        
        0,                   
        &size,         
        MEM_RESERVE | MEM_COMMIT, 
        PAGE_READWRITE 
    );
    if (!NT_SUCCESS(status)) {
        //printf("NtAllocateVirtualMemory failed: 0x%X\n", status);
        return NULL;
    }
    return baseAddress;
}

BOOL ZivFreeMemory(void* address, size_t size) {
    NTSTATUS status = NtFreeVirtualMemory(
        NtCurrentProcess(), 
        &address, 
        &size, 
        MEM_RELEASE
    );
    if (!NT_SUCCESS(status)) {
        //printf("NtFreeVirtualMemory failed: 0x%X\n", status);
        return FALSE;
    }
    return TRUE;
}


NTSTATUS NtSleep(int seconds){
    int time = seconds * 10000000;
    time = time * -1;
    LARGE_INTEGER delay;
    delay.QuadPart = time;
    return NtDelayExecution(FALSE, &delay);
}


WCHAR* NtItoW(int num, WCHAR* str) {
    int i = 0;
    int isNegative = 0;
    if (num < 0) {
        isNegative = 1;
        num = -num;
    }
    do {
        str[i++] = num % 10 + L'0'; 
        num /= 10;
    } while (num > 0);
    if (isNegative) {
        str[i++] = L'-'; 
    }

    int start = 0;
    int end = i - 1;
    while (start < end) {
        WCHAR temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
    str[i] = L'\0'; 

    return str;
}

WCHAR* NtStatusToWString(NTSTATUS status, WCHAR* str) {
    const int HEX_BASE = 16;
    const WCHAR* HEX_DIGITS = L"0123456789ABCDEF";
    int i = 0;
    str[i++] = L'0';
    str[i++] = L'x';
    for (int shift = 28; shift >= 0; shift -= 4) {
        int nibble = (status >> shift) & 0xF; // I still don't understand how this works, but don't ask me please
        str[i++] = HEX_DIGITS[nibble];
    }
    str[i] = L'\0';
    return str;
}



NTSTATUS NtOpenKeyboard(PHANDLE KeyboardHandle) {
    NTSTATUS status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN enabled;
    UNICODE_STRING KeyboardName = RTL_CONSTANT_STRING(L"\\Device\\KeyboardClass0");

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyboardName,
                               OBJ_KERNEL_HANDLE,  
                               NULL,
                               NULL);
    RtlAdjustPrivilege(SE_LOAD_DRIVER_PRIVILEGE | SE_DEBUG_PRIVILEGE, TRUE, FALSE, &enabled);
    status = NtCreateFile(KeyboardHandle,
                          SYNCHRONIZE | GENERIC_READ | FILE_READ_ATTRIBUTES,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_OPEN,
                          FILE_DIRECTORY_FILE,
                          NULL,
                          0);                 

    if (!NT_SUCCESS(status)) {
        ZivPrintNtStatus(L"NtCreateFile", status);
        return status;
    }

    return status;
}







void WaitForKeyPress(HANDLE hKeyboard) {
    IO_STATUS_BLOCK ioStatusBlock;
    WCHAR* buffer = ZivAllocateMemory(sizeof(KEYBOARD_INPUT_DATA) * sizeof(WCHAR));
    LARGE_INTEGER byteOffset = { 0 };
    NTSTATUS status = NtReadFile(hKeyboard, 
                                NULL, 
                                NULL, 
                                NULL, 
                                &ioStatusBlock , 
                                buffer, 
                                sizeof(KEYBOARD_INPUT_DATA) * sizeof(WCHAR), 
                                &byteOffset, 
                                NULL);
    if(!NT_SUCCESS(status)){
        ZivPrintNtStatus(L"NtReadFile", status);
    }
    status = NtWaitForSingleObject(hKeyboard, FALSE, NULL);
    if(!NT_SUCCESS(status)){
        ZivPrintNtStatus(L"NtWaitForSingleObject", status);
    }
    if(ioStatusBlock.Information > 0){
        NtPuts(L"Key Pressed");
    }
}

