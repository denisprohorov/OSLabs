#include <iostream>
#include "Vector.h"

int main()
{
    Number n1(100);
    Number n2 = createNumber(10);
    std::cout << "\nNumber1: " << n1.toString() << ", Number2: " << n2.toString() << "\n";
    std::cout << "+: " << (n1 + n2).toString() << "\n";
    std::cout << "-: " << (n1 - n2).toString() << "\n";
    std::cout << "*: " << (n1 * n2).toString() << "\n";
    std::cout << "/: " << (n1 / n2).toString() <<"\n";

    std::cout << "\n--------------------------\n";

    Vector v1(3, 4);
    Vector v2(-3, 4);
    Vector v3 = v1 + v2;
    std::cout << "\nVector1: (3, 4), Vector2: (-3, 4)\n";
    std::cout << "\nv3 = v1 + v2:" << v3.toString() << '\n';
    std::cout << "R vector1: " << v1.r().toString() << "\n";
    std::cout << "fi vector1: " << v1.fi().toString() << "\n";
    std::cout << "R vector2: " << v2.r().toString() << "\n";
    std::cout << "fi vector2: " << v2.fi().toString() << "\n";

    std::cout << "\n--------------------------\n";

    std::cout << "\nPress Enter to close this window.\n";
    std::cin.get();
}