
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
#include "stdafx.h"

#include "Camera.h"

using namespace Skin;
using namespace Utils;

Camera::Camera(const Vector& vecEye, const Vector& vecLookAt, const Vector& vecUp)
	: m_vecEye(vecEye), m_vecLookAt(vecLookAt), m_vecUp(vecUp),
      m_dRestrictionMin(1.0), m_dRestrictionMax(10.0)
{

}

void Camera::setView(const Vector& vecEye, const Vector& vecLookAt, const Vector& vecUp) {
	m_vecEye = vecEye;
	m_vecLookAt = vecLookAt;
	m_vecUp = vecUp;
}

void Camera::look(Vector& vecEye, Vector& vecLookAt, Vector& vecUp) const {
	vecEye = m_vecEye;
	vecLookAt = m_vecLookAt;
	vecUp = m_vecUp;
}

void Camera::changeView(double hAngleDelta, double vAngleDelta) {
	hAngleDelta = -hAngleDelta;
	vAngleDelta = +vAngleDelta;

	Vector vLookDir = m_vecLookAt - m_vecEye;
	double vAngle = angle(vLookDir, m_vecUp);
	double hAngle = atan(vLookDir.y / vLookDir.x);
	if (vLookDir.x < 0.0f)
		hAngle += Math::PI;

	hAngle -= hAngleDelta;
	while (hAngle < 0.0)
		hAngle += 2 * Math::PI;
	while (hAngle > 2 * Math::PI) {
		hAngle -= 2 * Math::PI;
	}

	vAngle += vAngleDelta;
	if (vAngle < Math::PI / 180.0)
		vAngle = Math::PI / 180.0;
	if (vAngle > Math::PI * (1.0 - 1.0 / 180.0))
		vAngle = Math::PI * (1.0 - 1.0 / 180.0);

	vLookDir.z = 1.0 * cos(vAngle);
	double len = 1.0 * sin(vAngle);
	vLookDir.x = len * cos(hAngle);
	vLookDir.y = len * sin(hAngle);

	double distance = (m_vecLookAt - m_vecEye).length();
	m_vecEye = m_vecLookAt - vLookDir * distance;
}

void Camera::moveView(double dist) {
	Vector vLookDir = m_vecLookAt - m_vecEye;
	dist = Math::clampValue(dist, vLookDir.length() - m_dRestrictionMax, vLookDir.length() - m_dRestrictionMin);
	Vector delta = vLookDir.normalize() * dist;
	m_vecEye += delta;
}

void Camera::applyRestriction() {
	Vector vLookDir = m_vecLookAt - m_vecEye;
	if (vLookDir.length() > m_dRestrictionMax)
		moveView(vLookDir.length() - m_dRestrictionMax);
	if (vLookDir.length() < m_dRestrictionMin)
		moveView(vLookDir.length() - m_dRestrictionMin);
}

void Camera::restrictView(double dMin, double dMax) {
	m_dRestrictionMin = dMin;
	m_dRestrictionMax = dMax;
	applyRestriction();
}
