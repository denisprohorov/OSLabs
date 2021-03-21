#include "Vector.h"
#include "Number.h"
#include <climits>
#define M_PI 3.14159265358979323846

Vector::Vector(int x, int y) : x(x), y(y){}
Vector::Vector(Number x, Number y) : x(x), y(y){}

Number Vector::r()
{
	Number num = x * x + y * y;
	return num.sqrt();
}

Number Vector::fi()
{
	if (x == 0) {
		if (y > 0) {
			return createNumber(M_PI / 2);
		}else if (y < 0) {
			return createNumber(-M_PI / 2);
		}else{
			return createNumber(INT_MAX);
		}
	} else {
		Number num = y / x;
		num = createNumber(atan(num.getValue()));
		if (x > 0) {
			return num;
		}
		if (x < 0 && !(y < 0)) {
			return num + createNumber(M_PI);
		}
		if (x < 0 && y < 0) {
			return num - createNumber(M_PI);
		}
	}
	return Number(0);
}
Vector Vector::operator+(Vector& v){
	return Vector(this->x + v.x, this->y + v.y);
}

std::string Vector::toString() {
    return ("vector{x = " + this->x.toString() + ", y = " + this->y.toString() + ";}");
}
