/**
 * 3D vector of type double
 */

#pragma once

#include <limits>

namespace Utils {

struct Vector {
	double x, y, z;
	Vector();
	Vector(double x, double y, double z);

	Vector& operator+=(const Vector& vec);
	Vector& operator-=(const Vector& vec);
	Vector& operator*=(double mul);
	Vector& operator/=(double div);
	const Vector operator+() const;
	const Vector operator-() const;

	double length() const;
	const Vector normalize() const;
	const Vector cross(const Vector& vec) const;

	const double* toArray() const { return (double*)this; }

	static const Vector ZERO;
};

const Vector operator+(const Vector& v1, const Vector& v2);
const Vector operator-(const Vector& v1, const Vector& v2);
const Vector operator*(const Vector& vec, double mul);
const Vector operator*(double mul, const Vector& vec);
const Vector operator/(const Vector& vec, double div);
double operator*(const Vector& v1, const Vector& v2);
const Vector cross(const Vector& v1, const Vector& v2);
double angle(const Vector& v1, const Vector& v2);
double shadow(const Vector& v1, const Vector& v2);
bool operator==(const Vector& v1, const Vector& v2);
bool operator!=(const Vector& v1, const Vector& v2);

struct Position : public Vector {
	Position() : Vector() {}
	Position(double theX, double theY, double theZ) 
		: Vector(theX, theY, theZ) {

	}

	Position(const Vector& vec) 
		: Vector(vec) {

	}

	double scaledX() const;
	double scaledY() const;
	double scaledZ() const;

	double distance(const Position& pos) const;

	static const double WORLD_SCALING;
	static const Position ORIGIN;
	static double distance(const Position& pos1, const Position& pos2);
};

inline bool doubleEquals(double lhs, double rhs, double epsilon = std::numeric_limits<float>::epsilon());

} // namespace Utils
