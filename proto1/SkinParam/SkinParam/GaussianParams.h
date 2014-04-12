/**
 * Sum of Gaussians param calculation
 */

#pragma once

#include "Utils/Color.h"
#include "Parallel/AbortableFuture.h"
#include <vector>

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
	private:
		ProfileSpace psp;
		RGBProfile sample(const float* params) const;
		RGBProfile nsample(int baseId, int nDims, int dim, const LerpStruct* lerps) const;
		void parseFile(const Utils::TString& filename);

		static GaussianParams getParamsFromRGBProfile(const RGBProfile& rgbProfile,
			const std::vector<float>& sigmas);
	};

};
