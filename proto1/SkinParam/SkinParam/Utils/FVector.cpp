
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
 * 3D vector of type float
 */
#include "FVector.h"
#include <cmath>
#include <algorithm>

using namespace Utils;

const FVector FVector::ZERO(0.0, 0.0, 0.0);

FVector::FVector() : x(0.0), y(0.0), z(0.0) {
}

FVector::FVector(float theX, float theY, float theZ)
	: x(theX), y(theY), z(theZ) {

}

FVector& FVector::operator+=(const FVector& vec) {
	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;
}

FVector& FVector::operator-=(const FVector& vec) {
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}

FVector& FVector::operator*=(float mul) {
	x *= mul;
	y *= mul;
	z *= mul;
	return *this;
}

FVector& FVector::operator/=(float div) {
	x /= div;
	y /= div;
	z /= div;
	return *this;
}

const FVector FVector::operator+() const {
	return *this;
}

const FVector FVector::operator-() const {
	return FVector(-x, -y, -z);
}

FVector::operator bool() const {
	return *this != ZERO;
}

float FVector::length() const {
	return sqrt(x * x + y * y + z * z);
}

const FVector FVector::normalize() const {
	return (*this) / length();
}

const FVector FVector::cross(const FVector& vec) const {
	float xx = y * vec.z - z * vec.y;
	float yy = z * vec.x - x * vec.z;
	float zz = x * vec.y - y * vec.x;
	return FVector(xx, yy, zz);
}

const FVector Utils::operator+(const FVector& v1, const FVector& v2) {
	return FVector(v1) += v2;
}

const FVector Utils::operator-(const FVector& v1, const FVector& v2) {
	return FVector(v1) -= v2;
}

const FVector Utils::operator*(const FVector& vec, float mul) {
	return FVector(vec) *= mul;
}

const FVector Utils::operator*(float mul, const FVector& vec) {
	return vec * mul;
}

const FVector Utils::operator/(const FVector& vec, float div) {
	return FVector(vec) /= div;
}

float Utils::operator*(const FVector& v1, const FVector& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

const FVector Utils::cross(const FVector& v1, const FVector& v2) {
	return v1.cross(v2);
}

float Utils::angle(const FVector& v1, const FVector& v2) {
	float cosValue = (v1 * v2) / (v1.length() * v2.length());
	if (cosValue > 1.0) cosValue = 1.0;
	else if (cosValue < -1.0) cosValue = -1.0;
	return acos(cosValue);
}

float Utils::shadow(const FVector& v1, const FVector& v2) {
	return v1 * v2 / v2.length();
}

bool Utils::floatEquals(float lhs, float rhs, float epsilon) {
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

bool Utils::isInfinite(float f) {
	static const float inf = std::numeric_limits<float>::infinity();
	return inf == f || inf == -f;
}

bool Utils::operator==(const FVector& v1, const FVector& v2) {
	const float EPSILON = 1e-6f;
	return floatEquals(v1.x, v2.x, EPSILON) && floatEquals(v1.y, v2.y, EPSILON)
		&& floatEquals(v1.z, v2.z, EPSILON);
}

bool Utils::operator!=(const FVector& v1, const FVector& v2) {
	return !operator==(v1, v2);
}
