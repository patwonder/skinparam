/**
 * Lighting
 */

#pragma once

#include "Vector.h"
#include "Color.h"

namespace Skin {
	struct Light {
		Utils::Vector vecPosition;
		Utils::Color coAmbient;
		Utils::Color coDiffuse;
		Utils::Color coSpecular;
		float fAtten0, fAtten1, fAtten2;

		Light(const Utils::Vector& vecPos,
			const Utils::Color& coAmbient, const Utils::Color& coDiffuse, const Utils::Color& coSpecular, 
			float fAtten0, float fAtten1, float fAtten2)
			: vecPosition(vecPos),
			  coAmbient(coAmbient), coDiffuse(coDiffuse), coSpecular(coSpecular),
			  fAtten0(fAtten0), fAtten1(fAtten1), fAtten2(fAtten2)
		{
		}

		bool isLit() const {
			return !(coAmbient.colorEquals(Utils::Color::Black) && coDiffuse.colorEquals(Utils::Color::Black)
				&& coSpecular.colorEquals(Utils::Color::Black));
		}
	};
} // namespace Skin
