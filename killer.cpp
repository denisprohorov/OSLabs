#include <iostream>
#include <signal.h>
#include <dirent.h>
#include <cstring>
#include <functional>
#include <unistd.h>
#include <sstream>

const int BUFFER_SIZE = 260;

bool ForEachProc(const std::function<bool(char *, int)> &Do);

bool IsExistByName(char *cmpValue) {
    std::cout << "proc with name " << cmpValue << " is Exist? " << std::boolalpha << ForEachProc(
            [&](char *name, int pid) {
                if (strcmp(name, cmpValue) == 0) {
                    return true;
                }
                return false;
            }) << '\n';
}

bool IsExistById(pid_t id) {
    std::cout << "proc with id " << id << " is Exist? " << std::boolalpha << (kill(id, SIGCONT) == 0) << '\n';
}

bool ForEachProc(const std::function<bool(char *, int)> &Do) {
    FILE *file;
    dirent *entry;
    char path[BUFFER_SIZE];
    int pid;

    DIR *dir = opendir("/proc");
    bool made = false;
    while ((entry = readdir(dir)) && !made) {
        if (!isdigit(entry->d_name[0])) {
            continue;
        }

        sprintf(path, "/proc/%s/stat", entry->d_name);
        file = fopen(path, "r");

        if (!file) {
            perror(path);
            continue;
        }

        fscanf(file, "%d %s", &pid, &path);
        auto name = new char[BUFFER_SIZE];
        strcpy(name, path + 1);
        name[strlen(path) - 2] = '\0';

        made = Do(name, pid);

        fclose(file);
    }
    closedir(dir);
    return made;
}

void KillByName(char nameCmp[]) {
    IsExistByName(nameCmp);
    ForEachProc([&](char *name, int pid) {
        if (strcmp(name, nameCmp) == 0) {
            kill(pid, SIGTERM);
        }
        return false;
    });
    sleep(1);
    IsExistByName(nameCmp);
}

int main(int argc, char *argv[]) {
    const char *name = "PROC_TO_KILL";
    char* buffer = getenv(name);
    if(buffer) {
        std::stringstream ss(buffer);
        while (ss.getline(buffer, 256, ',')) {
            KillByName(buffer);
        }
    }
    if (argc >= 3) {
        if (strcmp(argv[1], "--name") == 0) {
            KillByName(argv[2]);
        }else if (strcmp(argv[1], "--id") == 0) {
            int id = atoi(argv[2]);
            IsExistById(id);
            kill(id, SIGTERM);
            sleep(1);
            IsExistById(id);
        }
    }
    return 0;
}

