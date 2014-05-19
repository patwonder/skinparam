
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

// Information about the near and far clipping plane

static const float DEPTH_NEAR = 0.1;
static const float DEPTH_FAR = 20.0;

static const float LIGHT_NEAR = 0.1;
static const float LIGHT_FAR = 20.0;

// converts from camera space depth to projection space depth
float getDepthPS(float depthCS) {
	return (depthCS - DEPTH_NEAR) / (DEPTH_FAR - DEPTH_NEAR);
}

// converts from projection space depth to camera space depth
float getDepthCS(float depthPS) {
	return lerp(DEPTH_NEAR, DEPTH_FAR, depthPS);
}

// converts from light space depth to light projection space depth
float getDepthPSL(float depthCSL) {
	return (depthCSL - LIGHT_NEAR) / (LIGHT_FAR - LIGHT_NEAR);
}

// converts from light projection space depth to light space depth
float getDepthCSL(float depthPSL) {
	return lerp(LIGHT_NEAR, LIGHT_FAR, depthPSL);
}
