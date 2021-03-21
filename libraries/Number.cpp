#include <iostream>
#include "Number.h"
#include <string>


Number::Number(int value) : value(value){}

Number Number::operator+(Number &&number){
    return Number(this->value + number.value);
}
Number Number::operator+(Number &number){
    return Number(this->value + number.value);
}
Number Number::operator-(Number &&number){
    return Number(this->value - number.value);
}
Number Number::operator-(Number &number){
    return Number(this->value - number.value);
}
Number Number::operator*(Number &number){
    return Number(this->value * number.value);
}
Number Number::operator/(Number &number){
    return Number(this->value / number.value);
}
bool Number::operator==(int value){ return (this->value == value);}
bool Number::operator>(int value){ return (this->value > value);}
bool Number::operator<(int value){ return (this->value < value);}
int Number::getValue(){return value;}
Number Number::sqrt(){
    this->value = std::sqrt(this->value);
    return *this;
}

std::string Number::toString() {return ("Number{value: " + std::to_string(this->value) + ";}");}

Number createNumber(int value)
{
	return Number(value);
}
