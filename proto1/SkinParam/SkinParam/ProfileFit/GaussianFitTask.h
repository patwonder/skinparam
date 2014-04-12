#pragma once

#include "spectrum.h"
#include "skincoeffs.h"
#include "Parallel/parallel.h"

namespace ProfileFit {

struct SpectralGaussianCoeffs {
	vector<SampledSpectrum> coeffs;
	vector<float> sigmas;
	SampledSpectrum error;
};

vector<Parallel::Task*> CreateGaussianFitTasks(const SkinCoefficients& coeffs,
	const vector<float>& sigmas, SpectralGaussianCoeffs& sgc);
void DestroyGaussianTasks(vector<Parallel::Task*>& tasks);
void ClearGaussianTasksCache();

} // namespace ProfileFit
