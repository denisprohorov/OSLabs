#include <iostream>
#include <windows.h>

int main(int argc, char *argv[]){
    GetCommandLineA();
    Sleep(2000);

    const CHAR *name = "PROC_TO_KILL";
    const DWORD buffSize = 65535;
    char buffer[buffSize];
    GetEnvironmentVariableA(name, buffer, buffSize);

    std::cout << buffer << '\n';
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << "\n";
    }
    return 0;
}

