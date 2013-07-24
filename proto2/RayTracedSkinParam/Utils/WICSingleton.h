// Declares the WICImagingFactory singleton object
#pragma once

#include <D2D1.h>
#include <wincodec.h>

namespace Utils {
	IWICImagingFactory* GetWIC();
} // namespace Utils
