/**
 * Contains data about the camera
 */
#include "stdafx.h"

#include "Camera.h"

using namespace Skin;
using namespace Utils;

Camera::Camera(const Vector& vecEye, const Vector& vecLookAt, const Vector& vecUp)
	: m_vecEye(vecEye), m_vecLookAt(vecLookAt), m_vecUp(vecUp)
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
