/*
 * Encapsulates an OpenRL texture object
 */

#pragma once

#include "RLUnknown.h"

namespace RLSkin {
	class Texture : public RLUnknown {
	private:
		RLtexture m_texture;
		Texture(const Utils::TString& strFileName, bool generateMips = true);
		Texture(const void* imageData, UINT width, UINT height, const WICPixelFormatGUID& formatGUID, bool generateMips = true);

		void create(const void* imageData, UINT width, UINT height, const WICPixelFormatGUID& formatGUID, bool generateMips);
		void shuffleData(void* imageData, UINT size, UINT stride, UINT pattern[4], UINT length);
	protected:
		~Texture() override;
	public:
		static void createInstance(const Utils::TString& strFileName, Texture** oppTexture, bool generateMips = true) {
			*oppTexture = new Texture(strFileName, generateMips);
		}
		static void createInstance(const void* imageData, UINT width, UINT height,
			const WICPixelFormatGUID& formatGUID, Texture** oppTexture, bool generateMips = true)
		{
			*oppTexture = new Texture(imageData, width, height, formatGUID, generateMips);
		}

		RLtexture getRLHandle() const { return m_texture; }
	};
} // namespace RLSkin
