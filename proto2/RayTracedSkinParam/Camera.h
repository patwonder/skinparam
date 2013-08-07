/**
 * Contains data about the camera
 */
#pragma once

namespace RLSkin {
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
