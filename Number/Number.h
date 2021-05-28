#pragma once
#ifndef OSLABS_NUMBER_H
#define OSLABS_NUMBER_H

#include <iostream>
#include <cmath>

class Number{
private:
    int value;

public:
    Number(int value);

    Number operator+(Number&& number);
    Number operator+(Number& number);
    Number operator-(Number&& number);
    Number operator-(Number& number);
    Number operator/(Number& number);
    Number operator*(Number& number);
    bool operator==(int value);
    bool operator>(int value);
    bool operator<(int value);
    Number sqrt();

    std::string toString();
    int getValue();

};

extern Number zero;
extern Number one;

Number createNumber(int value);


#endif //OSLABS_NUMBER_H
