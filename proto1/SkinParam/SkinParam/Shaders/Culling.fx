
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
 * Utility to do frustrum culling
 */

//--------------------------------------------------------------------------------------
// Returns the distance of a given point from a given plane
//--------------------------------------------------------------------------------------
float DistanceFromPlane ( 
                        float3 f3Position,      // World space position of the patch control point
                        float4 f4PlaneEquation  // Plane equation of a frustum plane
                        )
{
    float fDistance = dot( float4( f3Position, 1.0f ), f4PlaneEquation );
    
    return fDistance;
}


//--------------------------------------------------------------------------------------
// Returns view frustum Culling test result (true / false)
//--------------------------------------------------------------------------------------
bool ViewFrustumCull(
                    float3 f3EdgePosition0,         // World space position of patch control point 0
                    float3 f3EdgePosition1,         // World space position of patch control point 1
                    float3 f3EdgePosition2,         // World space position of patch control point 2
                    float4 f4ViewFrustumPlanes[4],  // 4 plane equations (left, right, top, bottom)
                    float fCullEpsilon              // Epsilon to determine the distance outside the view frustum is still considered inside
                    )
{    
    float4 f4PlaneTest;
    float fPlaneTest;
    
    // Left clip plane
    f4PlaneTest.x = ( ( DistanceFromPlane( f3EdgePosition0, f4ViewFrustumPlanes[0]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition1, f4ViewFrustumPlanes[0]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition2, f4ViewFrustumPlanes[0]) > -fCullEpsilon ) ? 1.0f : 0.0f );
    // Right clip plane
    f4PlaneTest.y = ( ( DistanceFromPlane( f3EdgePosition0, f4ViewFrustumPlanes[1]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition1, f4ViewFrustumPlanes[1]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition2, f4ViewFrustumPlanes[1]) > -fCullEpsilon ) ? 1.0f : 0.0f );
    // Top clip plane
    f4PlaneTest.z = ( ( DistanceFromPlane( f3EdgePosition0, f4ViewFrustumPlanes[2]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition1, f4ViewFrustumPlanes[2]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition2, f4ViewFrustumPlanes[2]) > -fCullEpsilon ) ? 1.0f : 0.0f );
    // Bottom clip plane
    f4PlaneTest.w = ( ( DistanceFromPlane( f3EdgePosition0, f4ViewFrustumPlanes[3]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition1, f4ViewFrustumPlanes[3]) > -fCullEpsilon ) ? 1.0f : 0.0f ) +
                    ( ( DistanceFromPlane( f3EdgePosition2, f4ViewFrustumPlanes[3]) > -fCullEpsilon ) ? 1.0f : 0.0f );
        
    // Triangle has to pass all 4 plane tests to be visible
    return !all( f4PlaneTest );
}

