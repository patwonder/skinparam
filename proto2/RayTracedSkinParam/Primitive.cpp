#include "stdafx.h"

#include "Primitive.h"
#include "Program.h"

using namespace RLSkin;

Primitive::Primitive() {
	rlGenPrimitives(1, &m_primitive);
	rlBindPrimitive(RL_PRIMITIVE, m_primitive);
}

Primitive::~Primitive() {
	rlDeletePrimitives(1, &m_primitive);
}

void Primitive::bind() {
	rlBindPrimitive(RL_PRIMITIVE, m_primitive);
}

void Primitive::useProgram(Program* program) {
	bind();
	program->use();
}
