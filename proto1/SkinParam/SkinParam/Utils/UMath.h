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
	} // namespace Math
} // namespace Utils
