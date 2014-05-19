
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

// Calculate the attenuation texture used to account for energy conservation

#include "Quad.fx"
#include "CookTorrance.fx"

float4 PS(PS_INPUT input) : SV_Target {
	float2 tex = input.texCoord;
	// Integrate the specular BRDF component over the hemisphere
	float costheta = tex.x;
	float pi = 3.14159265358979324;
	float m = tex.y;
	float sum = 0.0;
	// More terms can be used for more accuracy
	int numterms = 200;
	float3 N = float3( 0.0, 0.0, 1.0 );
	float3 V = float3(0.0, sqrt( 1.0 - costheta*costheta ), costheta);
	for( int i = 0; i < numterms; i++ ) {
		float phip = float(i) / float( numterms - 1 ) * 2.0 * pi;
		float localsum = 0.0f;
		float cosp = cos( phip );
		float sinp = sin( phip );
		for( int j = 0; j < numterms; j++ ) {
			float thetap = float(j) / float( numterms - 1 ) * pi / 2.0;
			float sint = sin( thetap );
			float cost = cos( thetap );
			float3 L = float3( sinp * sint, cosp * sint, cost );
			localsum += CookTorrance( N, V, L, normalize(V + L), m ) * sint;
		}
		sum += localsum * (pi / 2.0) / float( numterms );
	}
	return sum * (2.0 * pi) / ( float( numterms ) );
}
