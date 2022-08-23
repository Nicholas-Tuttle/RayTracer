#pragma once

#include "png.h"
#include <algorithm>

namespace RayTracer
{
	class Color
	{
	private:
		png_byte r;
		png_byte g;
		png_byte b;
		png_byte a;

		float float_r;
		float float_g;
		float float_b;
		float float_a;

	public:
		const static png_byte MAX_COLOR = 0xFF;
		Color() : Color(MAX_COLOR, MAX_COLOR, MAX_COLOR, MAX_COLOR) {}
		Color(png_byte R, png_byte G, png_byte B, png_byte A)
			: r(R), g(G), b(B), a(A), 
			float_r((float)R / MAX_COLOR), float_g((float)G / MAX_COLOR), 
			float_b((float)B / MAX_COLOR), float_a((float)A / MAX_COLOR) 
		{};

		Color(float R, float G, float B, float A)
			: 
			r((png_byte)(std::max<float>(0.0f, std::min<float>(1.0f, abs(R))) * MAX_COLOR)),
			g((png_byte)(std::max<float>(0.0f, std::min<float>(1.0f, abs(G))) * MAX_COLOR)),
			b((png_byte)(std::max<float>(0.0f, std::min<float>(1.0f, abs(B))) * MAX_COLOR)),
			a((png_byte)(std::max<float>(0.0f, std::min<float>(1.0f, abs(A))) * MAX_COLOR)),
			float_r(R), float_g(G), float_b(B), float_a(A)
		{};

		void operator=(const Color &color)
		{
			r = color.r;
			g = color.g;
			b = color.b;
			a = color.a;

			float_r = color.float_r;
			float_g = color.float_g;
			float_b = color.float_b;
			float_a = color.float_a;
		}

		void operator*=(const Color &color)
		{
			float_r *= color.float_r;
			float_g *= color.float_g;
			float_b *= color.float_b;
			float_a *= color.float_a;

			r = (png_byte)std::min<float>((float)MAX_COLOR, float_r * MAX_COLOR);
			g = (png_byte)std::min<float>((float)MAX_COLOR, float_g * MAX_COLOR);
			b = (png_byte)std::min<float>((float)MAX_COLOR, float_b * MAX_COLOR);
			a = (png_byte)(float_a * MAX_COLOR);
		}

		Color operator*(const Color& color) const
		{
			return Color(color.float_r * float_r, color.float_g * float_g, color.float_b * float_b, color.float_a * float_a);
		}

		Color operator*(float scalar) const
		{
			return Color(scalar * float_r, scalar * float_g, scalar * float_b, std::max(0.0f, std::min<float>(scalar * float_a, 1.0f)));
		}

		Color operator+(const Color &color) const
		{
			return Color(float_r + color.float_r, float_g + color.float_g, float_b + color.float_b, 
				std::max(0.0f, std::min<float>(float_a + color.float_a, 1.0f)));
		}

		png_byte R() const { return r; }
		png_byte G() const { return g; }
		png_byte B() const { return b; }
		png_byte A() const { return a; }

		float R_float() const { return float_r; }
		float G_float() const { return float_g; }
		float B_float() const { return float_b; }
		float A_float() const { return float_a; }

		Color Modulate(const Color &other, float modularity) const
		{
			float actual_modularity = std::max<float>(0, std::min<float>(modularity, 1.0f));
			return Color(*this * (1.0f - actual_modularity) + other * actual_modularity);
		}
	};
}

