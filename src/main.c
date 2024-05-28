#include "ntstdio.h"
#include <winnt.h>
#include <winternl.h>




void _entry() {
    NtPuts(L"Press Any Key\n");
    HANDLE hKeyboard;
    HANDLE hEvent;
    NtOpenKeyboard(&hKeyboard);
    while(1){
        NtCreateKeyboardEvent(&hEvent);
        NtSleepMilliseconds(9);
        GetKeyPress(hKeyboard, hEvent);
        NtClose(hEvent);
    }
}

