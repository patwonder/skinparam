// Frustum culling

#pragma once

namespace Skin{

	class Frustum {
	public:
		Frustum();
		~Frustum();
		// Call this every time the camera moves to update the frustum
		void CalculateFrustum(const XMMATRIX& ViewProjectMatrix, XMFLOAT4* pFrustumPlanes = nullptr);
		// This takes a 3D point and returns TRUE if it's inside of the frustum
		bool PointInFrustum(const XMVECTOR& Point) const;
		// tests if a sphere intersects the frustum
		bool SphereIntersectsFrustum(const XMVECTOR& Center, float Radius) const;
		// tests if a box intersects the frustrum
		bool BoxIntersectFrustum(XMVECTOR Points[8]) const;
	private:
		// This holds the A B C and D values for each side of our frustum.
		XMFLOAT4 FrustumPlane[6];
	};

}
