/**
 * Contains data about the camera
 */
#include "stdafx.h"

#include "Camera.h"

using namespace RLSkin;
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
