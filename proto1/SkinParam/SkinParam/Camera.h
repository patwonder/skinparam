
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
 * Contains data about the camera
 */
#pragma once

#include "Vector.h"

namespace Skin {
	class Camera {
	private:
		Utils::Vector m_vecEye;
		Utils::Vector m_vecLookAt;
		Utils::Vector m_vecUp;
		double m_dRestrictionMin;
		double m_dRestrictionMax;

		void applyRestriction();
	public:
		Camera(const Utils::Vector& vecEye, const Utils::Vector& vecLookAt, const Utils::Vector& vecUp);

		void setView(const Utils::Vector& vecEye, const Utils::Vector& vecLookAt, const Utils::Vector& vecUp);

		void look(Utils::Vector& vecEye, Utils::Vector& vecLookAt, Utils::Vector& vecUp) const;

		void moveTo(const Utils::Vector& vPos) {
			m_vecEye = vPos;
		}

		void lookTo(const Utils::Vector& vOri) {
			m_vecLookAt = vOri;
		}

		void changeView(double hAngleDelta, double vAngleDelta);
		void moveView(double dist);
		void restrictView(double dMin, double dMax);

		const Utils::Vector& getVecEye() const { return m_vecEye; }
		const Utils::Vector& getVecLookAt() const { return m_vecLookAt; }
		const Utils::Vector& getVecUp() const { return m_vecUp; }
	};
} // namespace Skin
