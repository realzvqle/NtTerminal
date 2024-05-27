#include "ntstdio.h"
#include <winnt.h>
#include <winternl.h>




void _entry() {
    NtPuts(L"Press Any Key\n");
    HANDLE hKeyboard;
    NtOpenKeyboard(&hKeyboard);
    WaitForKeyPress(hKeyboard);
    
    //NtPuts(L"idk");
    while(1){continue;}
}

