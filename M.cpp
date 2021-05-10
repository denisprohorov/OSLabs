#include <iostream>
#include <unistd.h>
#include "constants.h"

const int end = -1;

int main(){
    int tmpValue;

    while(tmpValue != END_OF_FILE){
        std::cin >> tmpValue;
        if (tmpValue == end) std::cout << END_OF_FILE << ' ';
        else std::cout << tmpValue * 7 << ' ';
    }

    return 0;
}