#include <iostream>
#include <windows.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

int main(){
    const CHAR *name = "PROC_TO_KILL";
    const CHAR *value = "notepad.exe,mspaint.exe";
    const DWORD buffSize = 65535;
    char buffer[buffSize];
    SetEnvironmentVariableA(name, value);
    GetEnvironmentVariableA(name, buffer, buffSize);

    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    std::cout << name << " { " << buffer << "};" << '\n';
    std::cout << "Create processes mspaint and notepad. It's important code have absolut path for this processes." <<
    "C:\\WINDOWS\\system32..." << "\n";
    CreateProcess("C:\\WINDOWS\\system32\\mspaint.exe", NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
    CreateProcess("C:\\WINDOWS\\system32\\notepad.exe", NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
    std::cout << "delete notepad throw id " << pi.dwProcessId << '\n';
    std::stringstream ss;
    ss << "Killer.exe ";
    ss << "--id ";
    ss << pi.dwProcessId;
    std::cout << ss.str().c_str() << '\n';

    CreateProcess(NULL, (LPSTR)ss.str().c_str(), NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);

    std::cout << "round 2" << '\n';
    std::cout << "Create processes mspaint and notepad. It's important code have absolut path for this processes." <<
    "C:\\WINDOWS\\system32..." << "\n";
    CreateProcess("C:\\WINDOWS\\system32\\mspaint.exe", NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
    CreateProcess("C:\\WINDOWS\\system32\\notepad.exe", NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
    std::cout << "delete explorer.exe throw name" << '\n';
    ss.str("");
    ss << "Killer.exe ";
    ss << "--name ";
    ss << "explorer.exe";
    std::cout << ss.str().c_str() << '\n';

    CreateProcess(NULL, (LPSTR)ss.str().c_str(), NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    std::cout << "it's normal explorer.exe just reload. Explorer.exe win" << '\n';

    SetEnvironmentVariableA(name, NULL);
    buffer[0] = '\0';
    GetEnvironmentVariableA(name, buffer, buffSize);
    std::cout << name << " { " << buffer << "};" << '\n';

    return 0;
}

