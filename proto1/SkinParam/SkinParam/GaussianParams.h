
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
 * Sum of Gaussians param calculation
 */

#pragma once

#include "Utils/Color.h"
#include "Parallel/AbortableFuture.h"
#include <vector>
#include <chrono>

namespace Skin {
	// Necessary structs to store the gaussian params
	typedef std::vector<float> SingleProfile;
	struct RGBProfile {
		SingleProfile red, green, blue;
	};
	struct SamplePoints {
		std::vector<float> points;
		int idMultiplier;
	};
	// The N-dimensional space of profiles
	struct ProfileSpace {
		std::vector<float> sigmas;
		std::vector<SamplePoints> paramSamplePoints;
		std::vector<RGBProfile> profiles;
	};

	struct GaussianParams {
	public:
		static const int NUM_GAUSSIANS = 6;
		float sigmas[NUM_GAUSSIANS];
		XMFLOAT3 coeffs[NUM_GAUSSIANS];
	};

	struct VariableParams {
		VariableParams(float f_mel, float f_eu, float f_blood, float f_ohg)
			: f_mel(f_mel), f_eu(f_eu), f_blood(f_blood), f_ohg(f_ohg) { }
		float f_mel, f_eu, f_blood, f_ohg;
	};

	struct LerpStruct;

	class GaussianParamsCalculator {
	public:
		typedef Parallel::AbortableFuture<GaussianParams,
			std::function<void()>, std::function<double()> > GaussianFuture;

		GaussianParamsCalculator(const Utils::TString& filename);
		GaussianParams getParams(const VariableParams& vps) const;
		GaussianFuture getLiveFitParams(const VariableParams& vps) const;
		std::chrono::microseconds perf() const;
	private:
		ProfileSpace psp;
		RGBProfile sample(const float* params) const;
		RGBProfile nsample(int baseId, int nDims, int dim, const LerpStruct* lerps) const;
		void parseFile(const Utils::TString& filename);

		static GaussianParams getParamsFromRGBProfile(const RGBProfile& rgbProfile,
			const std::vector<float>& sigmas);
	};

};
