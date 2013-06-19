/**
 * 3D vector of type double
 */
#include "Vector.h"
#include <cmath>
#include <algorithm>

using namespace Utils;

const Vector Vector::ZERO(0.0, 0.0, 0.0);

Vector::Vector() : x(0.0), y(0.0), z(0.0) {
}

Vector::Vector(double theX, double theY, double theZ)
	: x(theX), y(theY), z(theZ) {

}

Vector& Vector::operator+=(const Vector& vec) {
	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;
}

Vector& Vector::operator-=(const Vector& vec) {
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}

Vector& Vector::operator*=(double mul) {
	x *= mul;
	y *= mul;
	z *= mul;
	return *this;
}

Vector& Vector::operator/=(double div) {
	x /= div;
	y /= div;
	z /= div;
	return *this;
}

const Vector Vector::operator+() const {
	return *this;
}

const Vector Vector::operator-() const {
	return Vector(-x, -y, -z);
}

Vector::operator bool() const {
	return *this != ZERO;
}

double Vector::length() const {
	return sqrt(x * x + y * y + z * z);
}

const Vector Vector::normalize() const {
	return (*this) / length();
}

const Vector Vector::cross(const Vector& vec) const {
	double xx = y * vec.z - z * vec.y;
	double yy = z * vec.x - x * vec.z;
	double zz = x * vec.y - y * vec.x;
	return Vector(xx, yy, zz);
}

const Vector Utils::operator+(const Vector& v1, const Vector& v2) {
	return Vector(v1) += v2;
}

const Vector Utils::operator-(const Vector& v1, const Vector& v2) {
	return Vector(v1) -= v2;
}

const Vector Utils::operator*(const Vector& vec, double mul) {
	return Vector(vec) *= mul;
}

const Vector Utils::operator*(double mul, const Vector& vec) {
	return vec * mul;
}

const Vector Utils::operator/(const Vector& vec, double div) {
	return Vector(vec) /= div;
}

double Utils::operator*(const Vector& v1, const Vector& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

const Vector Utils::cross(const Vector& v1, const Vector& v2) {
	return v1.cross(v2);
}

double Utils::angle(const Vector& v1, const Vector& v2) {
	double cosValue = (v1 * v2) / (v1.length() * v2.length());
	if (cosValue > 1.0) cosValue = 1.0;
	else if (cosValue < -1.0) cosValue = -1.0;
	return acos(cosValue);
}

double Utils::shadow(const Vector& v1, const Vector& v2) {
	return v1 * v2 / v2.length();
}

inline bool Utils::doubleEquals(double lhs, double rhs, double epsilon) {
	if(fabs(lhs) < fabs(rhs)) {
		// Always use fabs(bigger/smaller - 1) < e to make sure the symmetry of equality.
		// i.e. fequal(a, b) should produce the same result as fequal(b, a).
		std::swap(lhs, rhs);
	}

	if(rhs == 0) {
		// for sake of "fequal(0, 0)"
		return lhs == 0;
	} else {
		// "equivalent" variation of "fabs(lhs / rhs - 1) < e", but will not "overflow".
		// because normally epsilon should be smaller than 1, so only "underflow " may occur.
		return abs(lhs - rhs) < epsilon * abs(rhs);
	}
}

bool Utils::operator==(const Vector& v1, const Vector& v2) {
	const double EPSILON = 1e-7;
	return doubleEquals(v1.x, v2.x, EPSILON) && doubleEquals(v1.y, v2.y, EPSILON)
		&& doubleEquals(v1.z, v2.z, EPSILON);
}

bool Utils::operator!=(const Vector& v1, const Vector& v2) {
	return !operator==(v1, v2);
}

const Position Position::ORIGIN(0.0, 0.0, 0.0);
const double Position::WORLD_SCALING = 1.0;

double Position::distance(const Position& pos) const {
	return distance(*this, pos);
}

double Position::distance(const Position &pos1, const Position &pos2) {
	double x = pos1.x - pos2.x;
	double y = pos1.y - pos2.y;
	double z = pos1.z - pos2.z;
	return sqrt(x * x + y * y + z * z);
}

double Position::scaledX() const {
	return x / WORLD_SCALING;
}

double Position::scaledY() const {
	return y / WORLD_SCALING;
}

double Position::scaledZ() const {
	return z / WORLD_SCALING;
}
