
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
 * Color
 */

#pragma once

namespace Utils {

	class Color {
	public:
		float red;
		float green;
		float blue;
		float alpha;

		Color(float r, float g, float b, float a)
			: red(r), green(g), blue(b), alpha(a) {}

		static const Color White;
		static const Color Black;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Aqua;
		static const Color Purple;
		static const Color Yellow;
		static const Color Transparent;

		// Scalar multiplication/division
		Color& operator*=(float multiplier) {
			red *= multiplier;
			green *= multiplier;
			blue *= multiplier;
			return *this;
		}
		Color& operator/=(float multiplier) {
			red /= multiplier;
			green /= multiplier;
			blue /= multiplier;
			return *this;
		}
		Color operator*(float multiplier) const {
			return Color(*this) *= multiplier;
		}
		Color operator/(float multiplier) const {
			return Color(*this) /= multiplier;
		}
		// Addition/subtraction
		Color& operator+=(const Color& color) {
			red += color.red;
			green += color.green;
			blue += color.blue;
			return *this;
		}
		Color& operator-=(const Color& color) {
			red -= color.red;
			green -= color.green;
			blue -= color.blue;
			return *this;
		}
		Color operator+(const Color& color) const {
			return Color(*this) += color;
		}
		Color operator-(const Color& color) const {
			return Color(*this) -= color;
		}
		// Sign operators
		Color operator-() const {
			return Color(-red, -green, -blue, alpha);
		}
		const Color& operator+() const {
			return *this;
		}
		// Color multiplication
		Color& operator*=(const Color& color) {
			red *= color.red;
			green *= color.green;
			blue *= color.blue;
			alpha *= color.alpha;
			return *this;
		}
		Color operator*(const Color& color) const {
			return Color(*this) *= color;
		}
		bool operator==(const Color& color) const {
			return colorEquals(color) && this->alpha == color.alpha;
		}
		bool colorEquals(const Color& color) const {
			return this->red == color.red && this->green == color.green && this->blue == color.blue;
		}
	};

	inline Color operator*(float multiplier, const Color& color) {
		return color * multiplier;
	}

} // namespace Color
