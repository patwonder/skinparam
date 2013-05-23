/**
 * Common math routines
 */

#pragma once

namespace Utils {
	namespace Math {
		extern const double PI;
		
		template <class T>
		inline T clampValue(T value, T min, T max) {
			return value < min ? min : (value > max ? max : value);
		}
		template <class T>
		inline T lerp(T min, T max, T lp) {
			return ((T)1 - lp) * min + lp * max;
		}
	} // namespace Math
} // namespace Utils
