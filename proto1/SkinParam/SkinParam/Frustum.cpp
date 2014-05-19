
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

#include "stdafx.h"

#include "Frustum.h"

using namespace Skin;

enum FrustumSide {RIGHT, LEFT, BOTTOM, TOP, FRONT, BACK};

Frustum::Frustum() {

}
Frustum::~Frustum() {

}

///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/// //
/// // This extracts our frustum from the projection and view matrix.
/// //
///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\* 
void Frustum::CalculateFrustum(const XMMATRIX& ViewProjMatrix, XMFLOAT4* pFrustumPlanes) {
	// right clipping plane 
	FrustumPlane[RIGHT].x = ViewProjMatrix._14 - ViewProjMatrix._11;
	FrustumPlane[RIGHT].y = ViewProjMatrix._24 - ViewProjMatrix._21;
	FrustumPlane[RIGHT].z = ViewProjMatrix._34 - ViewProjMatrix._31;
	FrustumPlane[RIGHT].w = ViewProjMatrix._44 - ViewProjMatrix._41;

	// normalize 
	XMStoreFloat4(FrustumPlane + RIGHT, XMPlaneNormalize(XMLoadFloat4(FrustumPlane + RIGHT)));

	// left clipping plane 
	FrustumPlane[LEFT].x = ViewProjMatrix._14 + ViewProjMatrix._11;
	FrustumPlane[LEFT].y = ViewProjMatrix._24 + ViewProjMatrix._21;
	FrustumPlane[LEFT].z = ViewProjMatrix._34 + ViewProjMatrix._31;
	FrustumPlane[LEFT].w = ViewProjMatrix._44 + ViewProjMatrix._41;

	// normalize 
	XMStoreFloat4(FrustumPlane + LEFT, XMPlaneNormalize(XMLoadFloat4(FrustumPlane + LEFT)));

	// bottom clipping plane 
	FrustumPlane[BOTTOM].x = ViewProjMatrix._14 + ViewProjMatrix._12;
	FrustumPlane[BOTTOM].y = ViewProjMatrix._24 + ViewProjMatrix._22;
	FrustumPlane[BOTTOM].z = ViewProjMatrix._34 + ViewProjMatrix._32;
	FrustumPlane[BOTTOM].w = ViewProjMatrix._44 + ViewProjMatrix._42;

	// normalize 
	XMStoreFloat4(FrustumPlane + BOTTOM, XMPlaneNormalize(XMLoadFloat4(FrustumPlane + BOTTOM)));

	// top clipping plane 
	FrustumPlane[TOP].x = ViewProjMatrix._14 - ViewProjMatrix._12;
	FrustumPlane[TOP].y = ViewProjMatrix._24 - ViewProjMatrix._22;
	FrustumPlane[TOP].z = ViewProjMatrix._34 - ViewProjMatrix._32;
	FrustumPlane[TOP].w = ViewProjMatrix._44 - ViewProjMatrix._42;

	// normalize 
	XMStoreFloat4(FrustumPlane + TOP, XMPlaneNormalize(XMLoadFloat4(FrustumPlane + TOP)));
	// near clipping plane 
	FrustumPlane[FRONT].x = ViewProjMatrix._14 + ViewProjMatrix._13;
	FrustumPlane[FRONT].y = ViewProjMatrix._24 + ViewProjMatrix._23;
	FrustumPlane[FRONT].z = ViewProjMatrix._34 + ViewProjMatrix._33;
	FrustumPlane[FRONT].w = ViewProjMatrix._44 + ViewProjMatrix._43;

	// normalize 
	XMStoreFloat4(FrustumPlane + FRONT, XMPlaneNormalize(XMLoadFloat4(FrustumPlane + FRONT)));

	// far clipping plane 
	FrustumPlane[BACK].x = ViewProjMatrix._14 - ViewProjMatrix._13;
	FrustumPlane[BACK].y = ViewProjMatrix._24 - ViewProjMatrix._23;
	FrustumPlane[BACK].z = ViewProjMatrix._34 - ViewProjMatrix._33;
	FrustumPlane[BACK].w = ViewProjMatrix._44 - ViewProjMatrix._43;

	// normalize 
	XMStoreFloat4(FrustumPlane + BACK, XMPlaneNormalize(XMLoadFloat4(FrustumPlane + BACK)));

	// output to parameter
	if (pFrustumPlanes) {
		memcpy(pFrustumPlanes, FrustumPlane, sizeof(FrustumPlane));
	}
}

///////////////////////////////// POINT IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/// //
/// // This determines if a point is inside of the frustum
/// //
///////////////////////////////// POINT IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\* 
bool Frustum::PointInFrustum(const XMVECTOR& Point) const {
	for (int i = 0; i < 6; i++) {
		float x = XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(FrustumPlane + i), Point));
		if (x < 0)
			return false;
	} 
	// The point was inside of the frustum (In front of ALL the sides of the frustum) 
	return true;
}

// tests if a sphere intersects the frustum
bool Frustum::SphereIntersectsFrustum(const XMVECTOR& Center, float Radius) const {
	for (int i = 0; i < 6; i++) {
		float x = XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(FrustumPlane + i), Center));
		if (x < -Radius)
			return false;
	}
	// The sphere intersects the frustum (In front of/partially intersects ALL the sides of the frustum) 
	return true;
}

// tests if a box intersects the frustrum
bool Frustum::BoxIntersectFrustum(XMVECTOR Points[8]) const {
	int iTotalIn = 0;

	// test all 8 corners against the 6 sides
	// if all points are behind 1 specific plane, we are out
	// if we are in with all points, then we are fully in
	for (int p = 0; p < 6; p++) {
		int iInCount = 8;
		int iPtIn = 1;

		for (int i = 0; i < 8; i++) {
			// test this point against the planes
			if(XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(FrustumPlane + p), Points[i])) < 0) {
				iPtIn = 0;
				--iInCount;
			}
		}

		// were all the points outside of plane p?
		if (iInCount == 0)
			return false;

		// check if they were all on the right side of the plane
		iTotalIn += iPtIn;
	}

	// so if iTotalIn is 6, then all are inside the view
	if(iTotalIn == 6)
		return true;

	// we must be partly in then otherwise
	return true;
}
