#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <iomanip>
#include <string>
#include <sstream>

//void printAllProcess(){
//    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//    PROCESSENTRY32 pe = { 0 };
//    pe.dwSize = sizeof(PROCESSENTRY32);
//
//    Process32First(hSnap, &pe);
//
//    do
//    {
//        std::cout << std::setw(30) << pe.szExeFile<< std::setw(10) << pe.th32ProcessID << std::setw(10) << pe.cntThreads << "\n";
//    }while (Process32Next(hSnap, &pe));
//    CloseHandle(hSnap);
//}

bool isExist(char* cmpValue, bool (*cmpPtr)(PROCESSENTRY32&, char*)){
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32);

    Process32First(hSnap, &pe);
    do
    {
        if(cmpPtr(pe, cmpValue)){
            CloseHandle(hSnap);
            return true;
        }
    }while (Process32Next(hSnap, &pe));
    CloseHandle(hSnap);
    return false;
}

void DeleteProcess(char* cmpValue, bool (*cmpPtr)(PROCESSENTRY32&, char*)){
    std::cout << "process " << cmpValue  << " before delete isExist? " <<
    std::boolalpha << isExist(cmpValue, cmpPtr) << '\n';
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32);

    HANDLE process;
    Process32First(hSnap, &pe);
    do
    {
        if(cmpPtr(pe, cmpValue)){
//            std::cout << pe.szExeFile;
            process = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pe.th32ProcessID);
            TerminateProcess(process, 0);
            WaitForSingleObject(process, INFINITE);// need time to delete
            CloseHandle(process);
        }
    }while (Process32Next(hSnap, &pe));
    CloseHandle(hSnap);
    std::cout << "process " << cmpValue  << " after party isExist? " <<
    std::boolalpha << isExist(cmpValue, cmpPtr) << '\n';
}

bool cmpByName(PROCESSENTRY32& pe, char* name){
    return (strcmp(pe.szExeFile, name) == 0);
}

bool cmpById(PROCESSENTRY32& pe, char* id){
    std::stringstream ss;
    ss << pe.th32ProcessID;
    return (strcmp(ss.str().c_str(), id) == 0);
}

int main(int argc, char *argv[]){
    GetCommandLineA();

    if(argc >= 3){
        if(strcmp(argv[1], "--name") == 0){
            DeleteProcess(argv[2], cmpByName);
        }
        if(strcmp(argv[1], "--id") == 0){
            DeleteProcess(argv[2], cmpById);
        }
    }
    const CHAR *name = "PROC_TO_KILL";
    const DWORD buffSize = 65535;
    char buffer[buffSize];
    GetEnvironmentVariableA(name, buffer, buffSize);
    std::stringstream ss(buffer);
    while(ss.getline(reinterpret_cast<char *>(&buffer), buffSize, ',')){
        DeleteProcess(buffer, cmpByName);
    }

    return 0;
}

