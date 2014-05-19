
/*
    Copyright(c) 2013-2014 Yifan Wu.

    This file is part of SkinParam.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

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
	operator bool () const;

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
