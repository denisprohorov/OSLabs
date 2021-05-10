#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int forkWithDup2(std::string path, int firstDup, int secondDup){
    int childPid;
    childPid = fork();
    if(childPid == 0){
        dup2(firstDup, 0);
        dup2(secondDup, 1);

        execl(path.c_str(), path.c_str(), NULL);
        exit(0);
    }
    return childPid;
}

int main(){
    std::cout << "Enter numbers" << '\n';
    int     outMInA[2];
    int     outAInP[2];
    int     outPInS[2];
    int childPid;
    pipe(outMInA);

    childPid = forkWithDup2("M", 0, outMInA[1]);

    pipe(outAInP);
    childPid = forkWithDup2("A", outMInA[0], outAInP[1]);

    pipe(outPInS);
    childPid = forkWithDup2("P", outAInP[0], outPInS[1]);

    childPid = forkWithDup2("S", outPInS[0], 1);

    waitpid(childPid, nullptr, 0);

    return 0;
}