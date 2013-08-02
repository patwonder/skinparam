#include "stdafx.h"

#include "Texture.h"
#include "D3DHelper.h"
#include "DirectXTex\DDSTextureLoader\DDSTextureLoader.h"

using namespace RLSkin;
using namespace Utils;
using namespace D3DHelper;

Texture::Texture(const TString& strFileName, bool generateMips) {
	void* imageData = nullptr;
	UINT width, height;
	WICPixelFormatGUID formatGUID;
	{
		TStringStream tss;
		tss << _T("Failed to load texture image data from \"") << strFileName << _T("\"");
		checkFailure(loadImageData(strFileName, &imageData, &width, &height, &formatGUID),
			tss.str());
	}
	create(imageData, width, height, formatGUID, generateMips);
	freeImageData(imageData);
}

Texture::Texture(const void* imageData, UINT width, UINT height,
				 const WICPixelFormatGUID& formatGUID, bool generateMips)
{
	create(imageData, width, height, formatGUID, generateMips);
}

void Texture::shuffleData(void* imageData, UINT size, UINT stride, UINT pattern[4], UINT length) {
	char* pCharView = (char*)imageData;
	char* pCharViewEnd = pCharView + size;
	char* cache = new char[stride * length];
	while (pCharView != pCharViewEnd) {
		for (UINT i = 0; i < stride; i++) {
			UINT p = pattern[i];
			for (UINT j = 0; j < length; j++) {
				cache[i * length + j] = pCharView[p * length + j];
			}
		}
		memcpy(pCharView, cache, stride * length);
		pCharView += stride * length;
	}
	delete [] cache;
}

void Texture::create(const void* imageData, UINT width, UINT height,
					 const WICPixelFormatGUID& formatGUID, bool generateMips)
{
	rlGenTextures(1, &m_texture);
	rlBindTexture(RL_TEXTURE_2D, m_texture);
	RLenum format, type;
	bool shuffle = false;
	UINT shuffle_stride = 0;
	UINT shuffle_pattern[4];
	UINT shuffle_length = 0;
	if (formatGUID == GUID_WICPixelFormat128bppRGBAFloat || formatGUID == GUID_WICPixelFormat128bppRGBFloat) {
		format = RL_RGBA;
		type = RL_FLOAT;
	} else if (formatGUID == GUID_WICPixelFormat32bppGrayFloat) {
		format = RL_LUMINANCE;
		type = RL_FLOAT;
	} else if (formatGUID == GUID_WICPixelFormat32bppRGBA) {
		format = RL_RGBA;
		type = RL_UNSIGNED_BYTE;
	} else if (formatGUID == GUID_WICPixelFormat24bppRGB) {
		format = RL_RGB;
		type = RL_UNSIGNED_BYTE;
	} else if (formatGUID == GUID_WICPixelFormat8bppGray) {
		format = RL_LUMINANCE;
		type = RL_UNSIGNED_BYTE;
	} else if (formatGUID == GUID_WICPixelFormat24bppBGR) {
		format = RL_RGB;
		type = RL_UNSIGNED_BYTE;
		// BGR->RGB
		shuffle = true;
		shuffle_stride = 3;
		shuffle_pattern[0] = 2;
		shuffle_pattern[1] = 1;
		shuffle_pattern[2] = 0;
		shuffle_length = 1;
	} else if (formatGUID == GUID_WICPixelFormat32bppBGR || formatGUID == GUID_WICPixelFormat32bppBGRA) {
		format = RL_RGBA;
		type = RL_UNSIGNED_BYTE;
		// BGR(A)->RGBA
		shuffle = true;
		shuffle_stride = 4;
		shuffle_pattern[0] = 2;
		shuffle_pattern[1] = 1;
		shuffle_pattern[2] = 0;
		shuffle_pattern[3] = 3;
		shuffle_length = 1;
	} else {
		checkFailure(E_UNEXPECTED, _T("Texture format not supported."));
	}

	void* newImageData = nullptr;
	if (shuffle) {
		UINT dataSize = width * height * shuffle_stride * shuffle_length;
		newImageData = malloc(dataSize);
		memcpy(newImageData, imageData, dataSize);
		shuffleData(newImageData, dataSize, shuffle_stride, shuffle_pattern, shuffle_length);
	}

	rlTexImage2D(RL_TEXTURE_2D, 0, format, width, height, 0, format, type, newImageData ? newImageData : imageData);
	if (newImageData)
		free(newImageData);

	if (generateMips)
		rlGenerateMipmap(RL_TEXTURE_2D);

	rlTexParameteri(RL_TEXTURE_2D, RL_TEXTURE_WRAP_R, RL_CLAMP_TO_EDGE);
	rlTexParameteri(RL_TEXTURE_2D, RL_TEXTURE_MIN_FILTER, RL_LINEAR_MIPMAP_LINEAR);
}

Texture::~Texture() {
	rlDeleteTextures(1, &m_texture);
}
