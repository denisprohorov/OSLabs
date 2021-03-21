#pragma once

#include <iostream>
#include "Number.h"

class Vector{
private:
    Number x;
    Number y;

public:
    Vector(int x, int y);
    Vector(Number x, Number y);

    std::string toString();
    Number r();
	Number fi();

	Vector operator+ (Vector& v);

};

