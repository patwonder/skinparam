/**
 * Material
 */

#pragma once;

#include "Color.h"

namespace Skin {
	struct Material {
		Utils::Color coAmbient;
		Utils::Color coDiffuse;
		Utils::Color coSpecular;
		Utils::Color coEmissive;
		float fShininess;
		float fBumpMultiplier;

		Material(const Utils::Color& coAmbient, const Utils::Color& coDiffuse, const Utils::Color& coSpecular,
			const Utils::Color& coEmissive, float fShininess, float fBumpMultiplier = 0.0f)
			: coAmbient(coAmbient), coDiffuse(coDiffuse), coSpecular(coSpecular), coEmissive(coEmissive),
			  fShininess(fShininess), fBumpMultiplier(fBumpMultiplier)
		{
		}

		static const Material White;
	};
} // namespace Skin
