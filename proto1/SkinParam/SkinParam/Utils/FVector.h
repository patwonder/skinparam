/**
 * 3D vector of type float
 */
#pragma once

#include <limits>

namespace Utils {

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

inline bool floatEquals(float lhs, float rhs, float epsilon = std::numeric_limits<float>::epsilon());

} // namespace Utils
