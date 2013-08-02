/*
 * Interface for drawable objects
 */

#pragma once

#include "Program.h"

namespace RLSkin {
	class Drawable {
	public:
		virtual void init() = 0;
		virtual void draw(Program* pProgram) = 0;
		virtual void cleanup() = 0;
		virtual ~Drawable() {};
		virtual void getBoundingSphere(Utils::FVector& oVecCenter, float& oRadius) const;
	};
} // namespace RLSkin
