#include "ntstdio.h"
#include <winnt.h>








NTSTATUS NtPuts(WCHAR* String){
    UNICODE_STRING uString;
    RtlInitUnicodeString(&uString, String);
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

NTSTATUS NtSleepMilliseconds(int milliseconds) {
    LARGE_INTEGER delay;
    delay.QuadPart = -((LONGLONG)milliseconds * 10000);
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

WCHAR* WCharToWCharPtr(WCHAR character) {
    WCHAR* str = (WCHAR*)ZivAllocateMemory(2 * sizeof(WCHAR));
    if (str != NULL) {
        str[0] = character;  
        str[1] = L'\0';      
    }
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

    // Create event for synchronization
           
    if (!NT_SUCCESS(status)) {
        ZivPrintNtStatus(L"NtCreateEvent", status);
        return status;
    }

    return status;
}

NTSTATUS NtCreateKeyboardEvent(PHANDLE EVENT){
    OBJECT_ATTRIBUTES ObjectAttributes;

    InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);
    NTSTATUS status = NtCreateEvent(EVENT, EVENT_ALL_ACCESS, &ObjectAttributes, NotificationEvent, FALSE);       
    if (!NT_SUCCESS(status)) {
        ZivPrintNtStatus(L"NtCreateEvent", status);
        return status;
    }
    return status;  
}

USHORT WaitForKeyPress(HANDLE hKeyboard, HANDLE hEvent) {
    static KEYBOARD_INPUT_DATA input;
    IO_STATUS_BLOCK ioStatusBlock;
    LARGE_INTEGER byteOffset = { 0 };
    
    NTSTATUS status = NtReadFile(hKeyboard, 
                                hEvent,  
                                NULL, 
                                NULL, 
                                &ioStatusBlock , 
                                &input, 
                                sizeof(KEYBOARD_INPUT_DATA), 
                                &byteOffset, 
                                NULL);
    if (!NT_SUCCESS(status)) {
        ZivPrintNtStatus(L"NtReadFile", status);
        return 0;
    }
    NtSleepMilliseconds(20);
    int prev = ioStatusBlock.Information;
    if (ioStatusBlock.Information > 0) {

        USHORT result = input.MakeCode;
        return result;
    } else {
        return 0;
    }
}

void GetKeyPress(HANDLE hKeyboard, HANDLE hEvent) {
    static USHORT prev_key = 0;
    USHORT key = WaitForKeyPress(hKeyboard, hEvent);
    WCHAR* idk = WCharToWCharPtr(MakeCodeToWChar(key));
    NtSleepMilliseconds(80);
    if(prev_key == key){
        return;
    }
    prev_key = key;
    NtPuts(idk);
    ZivFreeMemory(idk, 2 * sizeof(WCHAR));
}

void GetKeyAndPrint(HANDLE hKeyboard){
    static HANDLE hEvent;
    NtCreateKeyboardEvent(&hEvent);
    GetKeyPress(hKeyboard, hEvent);
    NtClose(hEvent);
    NtSleepMilliseconds(200);
}

USHORT WCharToMakeCode(WCHAR character) {
    switch (character) {
        case L'`': return 41;
        case L'1': return 2;
        case L'2': return 3;
        case L'3': return 4;
        case L'4': return 5;
        case L'5': return 6;
        case L'6': return 7;
        case L'7': return 8;
        case L'8': return 9;
        case L'9': return 10;
        case L'0': return 11;
        case L'-': return 12;
        case L'=': return 13;
        case L'\b': return 14; 
        case L'\t': return 15; 
        case L'q': return 16;
        case L'w': return 17;
        case L'e': return 18;
        case L'r': return 19;
        case L't': return 20;
        case L'y': return 21;
        case L'u': return 22;
        case L'i': return 23;
        case L'o': return 24;
        case L'p': return 25;
        case L'[': return 26;
        case L']': return 27;
        case L'\n': return 28; 
        case L'a': return 30;
        case L's': return 31;
        case L'd': return 32;
        case L'f': return 33;
        case L'g': return 34;
        case L'h': return 35;
        case L'j': return 36;
        case L'k': return 37;
        case L'l': return 38;
        case L';': return 39;
        case L'\'': return 40;
        case L'\\': return 43;
        case L'z': return 44;
        case L'x': return 45;
        case L'c': return 46;
        case L'v': return 47;
        case L'b': return 48;
        case L'n': return 49;
        case L'm': return 50;
        case L',': return 51;
        case L'.': return 52;
        case L'/': return 53;
        case L' ': return 57; 
        case L'A': return 30; 
        case L'B': return 48;
        case L'C': return 46;
        case L'D': return 32;
        case L'E': return 18;
        case L'F': return 33;
        case L'G': return 34;
        case L'H': return 35;
        case L'I': return 23;
        case L'J': return 36;
        case L'K': return 37;
        case L'L': return 38;
        case L'M': return 50;
        case L'N': return 49;
        case L'O': return 24;
        case L'P': return 25;
        case L'Q': return 16;
        case L'R': return 19;
        case L'S': return 31;
        case L'T': return 20;
        case L'U': return 22;
        case L'V': return 47;
        case L'W': return 17;
        case L'X': return 45;
        case L'Y': return 21;
        case L'Z': return 44;
        default: return 0; 
    }
}

WCHAR MakeCodeToWChar(USHORT makeCode) {
    switch (makeCode) {
        case 41: return L'`';
        case 2: return L'1';
        case 3: return L'2';
        case 4: return L'3';
        case 5: return L'4';
        case 6: return L'5';
        case 7: return L'6';
        case 8: return L'7';
        case 9: return L'8';
        case 10: return L'9';
        case 11: return L'0';
        case 12: return L'-';
        case 13: return L'=';
        case 14: return L'\b'; 
        case 15: return L'\t'; 
        case 16: return L'q';
        case 17: return L'w';
        case 18: return L'e';
        case 19: return L'r';
        case 20: return L't';
        case 21: return L'y';
        case 22: return L'u';
        case 23: return L'i';
        case 24: return L'o';
        case 25: return L'p';
        case 26: return L'[';
        case 27: return L']';
        case 28: return L'\n'; 
        case 30: return L'a';
        case 31: return L's';
        case 32: return L'd';
        case 33: return L'f';
        case 34: return L'g';
        case 35: return L'h';
        case 36: return L'j';
        case 37: return L'k';
        case 38: return L'l';
        case 39: return L';';
        case 40: return L'\'';
        case 43: return L'\\';
        case 44: return L'z';
        case 45: return L'x';
        case 46: return L'c';
        case 47: return L'v';
        case 48: return L'b';
        case 49: return L'n';
        case 50: return L'm';
        case 51: return L',';
        case 52: return L'.';
        case 53: return L'/';
        case 57: return L' '; 
        default: return L'\0'; 
    }
}

