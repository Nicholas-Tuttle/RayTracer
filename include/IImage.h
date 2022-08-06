#pragma once

#include "png.h"
#include "Pixel.h"
#include <vector>

namespace RayTracer
{
	class IImage
	{
	public:
		virtual bool SetPixelColor(size_t x, size_t y, const Color &value) = 0;
		virtual const std::vector<std::vector<png_byte>>& GetColorRGBAValues() const = 0;
	};
}
