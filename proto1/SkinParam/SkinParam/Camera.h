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
	public:
		Camera(const Utils::Vector& vecEye, const Utils::Vector& vecLookAt, const Utils::Vector& vecUp);

		void setView(const Utils::Vector& vecEye, const Utils::Vector& vecLookAt, const Utils::Vector& vecUp);

		void look(Utils::Vector& vecEye, Utils::Vector& vecLookAt, Utils::Vector& vecUp) const;

		const Utils::Vector& getVecEye() const { return m_vecEye; }
		const Utils::Vector& getVecLookAt() const { return m_vecLookAt; }
		const Utils::Vector& getVecUp() const { return m_vecUp; }
	};
} // namespace Skin
