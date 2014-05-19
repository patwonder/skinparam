
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
#pragma once

#include <limits>

namespace Utils {

bool floatEquals(float lhs, float rhs, float epsilon = std::numeric_limits<float>::epsilon());
bool isInfinite(float f);

struct FVector {
	float x, y, z;
	FVector();
	FVector(float x, float y, float z);

	FVector& operator+=(const FVector& vec);
	FVector& operator-=(const FVector& vec);
	FVector& operator*=(float mul);
	FVector& operator/=(float div);
	const FVector operator+() const;
	const FVector operator-() const;
	operator bool () const;

	float length() const;
	const FVector normalize() const;
	const FVector cross(const FVector& vec) const;

	const float* toArray() const { return (float*)this; }

	bool isNaN() const { return x != x || y != y || z != z || isInfinite(x) || isInfinite(y) || isInfinite(z); }
	void fixNaN() { if (isNaN()) *this = FVector(0.0f, 0.0f, 1.0f); }

	static const FVector ZERO;
};

const FVector operator+(const FVector& v1, const FVector& v2);
const FVector operator-(const FVector& v1, const FVector& v2);
const FVector operator*(const FVector& vec, float mul);
const FVector operator*(float mul, const FVector& vec);
const FVector operator/(const FVector& vec, float div);
float operator*(const FVector& v1, const FVector& v2);
const FVector cross(const FVector& v1, const FVector& v2);
float angle(const FVector& v1, const FVector& v2);
float shadow(const FVector& v1, const FVector& v2);
bool operator==(const FVector& v1, const FVector& v2);
bool operator!=(const FVector& v1, const FVector& v2);

} // namespace Utils
