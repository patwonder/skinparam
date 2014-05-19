
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
