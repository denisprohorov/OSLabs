#pragma once
#ifdef Vector_EXPORTS
#define VECTOR_API __declspec(dllexport)
#else
#define VECTOR_API __declspec(dllimport)
#endif

#include <iostream>
#include "Number.h"

VECTOR_API class Vector{
private:
    Number x;
    Number y;

public:
    VECTOR_API Vector(int x, int y);
    VECTOR_API Vector(Number x, Number y);

    VECTOR_API std::string toString();
    VECTOR_API Number r();
	VECTOR_API Number fi();

	VECTOR_API Vector operator+ (Vector& v);

};

