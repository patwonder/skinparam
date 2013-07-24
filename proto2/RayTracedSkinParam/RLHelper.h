/*
 * Helper functions for OpenRL
 */

#pragma once

namespace RLSkin { namespace RLHelper {
	void checkFailure(HRESULT hr, const Utils::TString& prompt);
	std::string readShaderSource(const Utils::TString& fileName);
} } // namespace RLSkin::RLHelper
