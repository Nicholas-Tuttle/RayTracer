#pragma once

#include "png.h"
#include "IImage.h"
#include <vector>

namespace RayTracer
{
	struct ImageResolution
	{
	public:
		ImageResolution(size_t x, size_t y)
			: X(x), Y(y) {};

		const size_t X;
		const size_t Y;
	};

	class Image : public IImage
	{
	private:
		ImageResolution resolution;

		std::vector<std::vector<png_byte>> Colors;
	public:
		Image(const ImageResolution &resolution)
			: resolution(resolution)
		{
			Colors.resize(resolution.Y);
			for (int height_index = 0; height_index < resolution.Y; height_index++)
			{
				// Mupltiply by 4 for RGBA
				Colors[height_index].resize(resolution.X * 4);
			}
		}

		virtual bool SetPixelColor(size_t x, size_t y, const Color &value) override
		{
			if (x < 0 || x >= resolution.X || y < 0 || y >= resolution.Y)
			{
				return false;
			}

			Colors[y][x * 4 + 0] = value.R();
			Colors[y][x * 4 + 1] = value.G();
			Colors[y][x * 4 + 2] = value.B();
			Colors[y][x * 4 + 3] = value.A();

			return true;
		}

		virtual const std::vector<std::vector<png_byte>>& GetColorRGBAValues() const override
		{
			return Colors;
		}
	};
}

