#include <iostream>
#include <windows.h>
#include <string>
//#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

int main(){
    const CHAR *name = "PROC_TO_KILL";
    const CHAR *value = "daddy";
    SetEnvironmentVariableA(name, value);

    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    if (!CreateProcess("Killer.exe", "hjjh ghj",
                       NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi))
    {
        wprintf(L"Can't create process!\n");
    }else{
        std::cout << "ok" << '\n';
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
    }

    SetEnvironmentVariableA(name, NULL);
    return 0;
}

