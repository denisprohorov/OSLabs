#include <iostream>
#include <unistd.h>
#include "constants.h"

int main(){
    int tmpValue;
    int sum = 0;

    while(tmpValue != END_OF_FILE){
        std::cin >> tmpValue;
        if (tmpValue == END_OF_FILE) break;
        else sum += tmpValue;
    }
    std::cout << "sum is " << sum << '\n';

    return 0;
}