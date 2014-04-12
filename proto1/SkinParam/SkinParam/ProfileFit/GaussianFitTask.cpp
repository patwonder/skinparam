#include "stdafx.h"

#include "GaussianFitTask.h"
#include "MultipoleProfileCalculator.h"
#include "gaussianfit.h"
#include <map>

using namespace Parallel;

namespace ProfileFit {

class GaussianFitTask : public Task {
public:
	static const int nLayers = 2;

	GaussianFitTask(const SampledSpectrum* mua, const SampledSpectrum* musp,
		const float* et, const float* thickness, SpectralGaussianCoeffs& coeffs,
		int sc, uint32_t desiredLength)
		: coeffs(coeffs), sc(sc), desiredLength(desiredLength)
	{
		for (int i = 0; i < nLayers; i++) {
			this->mua[i] = mua[i][sc];
			this->musp[i] = musp[i][sc];
			this->et[i] = et[i];
			this->thickness[i] = thickness[i];
		}
	}

private:
	float mua[nLayers];
	float musp[nLayers];
	float et[nLayers];
	float thickness[nLayers];
	SpectralGaussianCoeffs& coeffs;
	uint32_t desiredLength;
	int sc;

	void Run() override;
	void DoGaussianFit(const MPC_Output* pOutput, float mfpMin, float mfpMax);
};

void GaussianFitTask::Run() {
	MPC_LayerSpec* pLayerSpecs = new MPC_LayerSpec[nLayers];
	MPC_Options options;
	options.desiredLength = desiredLength;
	options.lerpOnThinSlab = true;

	// Compute mfp
	float mfpMin = FLT_MAX;
	float mfpMax = 0.f;
	float mfpTotal = 0.f;
	for (int layer = 0; layer < nLayers; layer++) {
		float mfp = 1.f / (mua[layer] + musp[layer]);
		mfpTotal += mfp;
		mfpMin = min(mfpMin, mfp);
		mfpMax = max(mfpMax, mfp);
	}
	float mfp = mfpTotal / (float)nLayers;
	
	// Fill in layer information
	for (int layer = 0; layer < nLayers; layer++) {
		MPC_LayerSpec& ls = pLayerSpecs[layer];
		ls.g_HG = 0.f;
		ls.ior = et[layer];
		ls.mua = mua[layer];
		ls.musp = musp[layer];
		ls.thickness = thickness[layer];
	}
	// Compute desired step size
	options.desiredStepSize = 12.f * mfp / (float)options.desiredLength;

	// Do the computation
	MPC_Output* pOutput;
	MPC_ComputeDiffusionProfile(nLayers, pLayerSpecs, &options, &pOutput);
	MPC_ResampleForUniformDistanceSquaredDistribution(pOutput, pOutput->length);

	// Do gaussian fit
	DoGaussianFit(pOutput, mfpMin, mfpMax);
		
	MPC_FreeOutput(pOutput);

	delete [] pLayerSpecs;
}

void GaussianFitTask::DoGaussianFit(const MPC_Output* pOutput, float mfpMin, float mfpMax) {
	const vector<float>& sigmas = coeffs.sigmas;

	int nSigmas = sigmas.size();
	const int nTargetSigmas = 6;
	const int sigmaNegExtent = nTargetSigmas / 2;
	const int sigmaPosExtent = nTargetSigmas - sigmaNegExtent - 1;

	// Compute the best fit of nTargetSigmas gaussians
	uint32 length = pOutput->length;
	vector<float> distArray(length);
	for (uint32 i = 0; i < length; i++) {
		float dsq = pOutput->pDistanceSquared[i];
		distArray[i] = sqrt(dsq);
	}
	GF_Output* pGFOutput;

	float minError = FLT_MAX;
	int minErrorSigmaCenter = 0;
	std::map<int, GF_Output*> oCache;
	// Try sigmas between mfpMin / 2 and mfpMax * 2
	for (int iSigmaCenter = sigmaNegExtent; iSigmaCenter < nSigmas - sigmaPosExtent; iSigmaCenter++) {
		float sigma = sigmas[iSigmaCenter];
		if (sigma < mfpMin / 2.f) continue;
		if (sigma > mfpMax * 2.f) break;
		GF_FitSumGaussians(length, &distArray[0], pOutput->pReflectance,
			nTargetSigmas, &sigmas[iSigmaCenter - sigmaNegExtent], &pGFOutput);
		if (pGFOutput->overallError < minError) {
			minError = pGFOutput->overallError;
			minErrorSigmaCenter = iSigmaCenter;
		}
		oCache[iSigmaCenter] = pGFOutput;
		pGFOutput = NULL;
	}

	// Use minErrorSigmaCenter to do fitting
	//GF_FitSumGaussians(length, &distArray[0], pOutput->pReflectance,
	//	nTargetSigmas, &sigmas[minErrorSigmaCenter - sigmaNegExtent], &pGFOutput);
	pGFOutput = oCache[minErrorSigmaCenter];

	for (int iSigma = minErrorSigmaCenter - sigmaNegExtent;
		iSigma <= minErrorSigmaCenter + sigmaPosExtent; iSigma++)
	{
		coeffs.coeffs[iSigma][sc] =
			pGFOutput->pNormalizedCoeffs[iSigma - (minErrorSigmaCenter - sigmaNegExtent)];
	}
	coeffs.error[sc] = pGFOutput->overallError;

	for (auto& pair : oCache)
		GF_FreeOutput(pair.second);
}

vector<Task*> CreateGaussianFitTasks(const SkinCoefficients& coeffs, const vector<float>& sigmas,
	SpectralGaussianCoeffs& sgc)
{
	WLDValue mfp_min(FLT_MAX), mfp_max(0.f);
	int nTotalTasks = 0;
	WLDValue mutp_epi = coeffs.mua_epi() + coeffs.musp_epi();
	WLDValue mutp_derm = coeffs.mua_derm() + coeffs.musp_derm();
	WLDValue mfp_epi = WLDValue(10) / mutp_epi;
	WLDValue mfp_derm = WLDValue(10) / mutp_derm;
	mfp_min = Min(Min(mfp_min, mfp_epi), mfp_derm);
	mfp_max = Max(Max(mfp_max, mfp_epi), mfp_derm);
	float minmfp = mfp_min.Min();
	float maxmfp = mfp_max.Max();

	sgc.sigmas = sigmas;

	SampledSpectrum mua[2] = {
		coeffs.mua_epi().toSampledSpectrum() / 10.f,
		coeffs.mua_derm().toSampledSpectrum() / 10.f
	};
	SampledSpectrum musp[2] = {
		coeffs.musp_epi().toSampledSpectrum() / 10.f,
		coeffs.musp_derm().toSampledSpectrum() / 10.f
	};
	float et[2] = { 1.4f, 1.4f };
	float thickness[2] = {
		0.25f, 20.f
	};

	sgc.coeffs.resize(sigmas.size(), SampledSpectrum(0.f));
	vector<Task*> tasks;
	for (int sc = 0; sc < SampledSpectrum::nComponents; sc++) {
		tasks.push_back(new GaussianFitTask(mua, musp, et, thickness,
			sgc, sc, 512));
	}
	return tasks;
}

void DestroyGaussianTasks(vector<Task*>& tasks) {
	for (Task* task : tasks)
		delete task;
	tasks.clear();
}

void ClearGaussianTasksCache() {
	MPC_ClearCache();
}

} // namespace ProfileFit
