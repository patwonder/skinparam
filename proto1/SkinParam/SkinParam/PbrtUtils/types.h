#pragma once

#pragma warning (disable : 4305) // double constant assigned to float
#pragma warning (disable : 4244) // int -> float conversion

#define PBRT_IS_WINDOWS

#include <float.h>
#include <vector>
#include <string>
#include <limits>

// Platform-specific definitions
#define isnan _isnan
#define isinf(f) (!_finite((f)))
#include <stdint.h>

namespace PbrtUtils {

using std::vector;
using std::string;

#ifdef NDEBUG
#define Assert(expr) ((void)0)
#else
#define Assert(expr) \
    ((expr) ? (void)0 : \
        Severe("Assertion \"%s\" failed in %s, line %d", \
               #expr, __FILE__, __LINE__))
#endif // NDEBUG

template <class scalar>
class ScalarTraits {
public:
	static scalar zero() {
		return value(0);
	}
	static scalar one() {
		return value(1);
	}
	template <class V>
	static scalar value(V v) {
		return scalar(v);
	}

	static scalar max() {
		return std::numeric_limits<scalar>::max();
	}
	static scalar negmax() {
		return std::numeric_limits<scalar>::lowest();
	}
	static bool isNaN(scalar value) {
		return isnan(value) ? true : false;
	}
};

#define ScaVal(v) ScalarTraits<scalar>::value(v)

// Global Inline Functions
inline float Lerp(float t, const float& v1, const float& v2) {
	return (1.f - t) * v1 + t * v2;
}

template <class scalar, class Entity>
inline Entity Lerp(scalar t, const Entity& v1, const Entity& v2) {
	return (ScalarTraits<scalar>::one() - t) * v1 + t * v2;
}


inline float Clamp(float val, float low, float high) {
    if (val < low) return low;
    else if (val > high) return high;
    else return val;
}


inline int Clamp(int val, int low, int high) {
    if (val < low) return low;
    else if (val > high) return high;
    else return val;
}

template <class scalar, class scalar1, class scalar2>
inline scalar Clamp(scalar val, scalar1 low, scalar2 high) {
    if (val < ScaVal(low)) return ScaVal(low);
    else if (val > ScaVal(high)) return ScaVal(high);
    else return val;
}

} // namespace PbrtUtils

#include "error.h"
