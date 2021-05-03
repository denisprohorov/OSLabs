#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include <unistd.h>
#include <wait.h>

pid_t CreateProc(char name[]){
    pid_t id = fork();
    if(id == 0){
        execl(name, NULL);
        exit(0);
    }else{
        return id;
    }
}

int main(){
    char* e[] = {"PROC_TO_KILL=proc1,proc2", NULL};
    std::cout << "create proc and proc1" << '\n';
    pid_t procId = CreateProc("proc");
    CreateProc("proc1");
    std::cout << "delete proc by id and proc1 by name throw variable PROC_TO_KILL" << '\n';
    pid_t id = fork();
    if(id == 0) {
        execle("killer", "d", "--id", std::to_string((int)procId).c_str(), NULL, e);
        std::cout << "can't load killer";
        exit(0);
    }
    //collect all zombie proc
    wait(nullptr);
    wait(nullptr);
//    wait(nullptr);
    waitpid(id, nullptr, 0);
    return 0;
}

