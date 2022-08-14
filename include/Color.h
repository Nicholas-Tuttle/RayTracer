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
		const static png_byte MAX_COLOR = 0xFF;

		float float_r;
		float float_g;
		float float_b;
		float float_a;

	public:
		Color() : Color(MAX_COLOR, MAX_COLOR, MAX_COLOR, MAX_COLOR) {}
		Color(png_byte R, png_byte G, png_byte B, png_byte A)
			: r(R), g(G), b(B), a(A), 
			float_r((float)R / MAX_COLOR), float_g((float)G / MAX_COLOR), 
			float_b((float)B / MAX_COLOR), float_a((float)A / MAX_COLOR) 
		{};

		Color(float R, float G, float B, float A)
			: 
			r(std::min<png_byte>(std::max<png_byte>(0, png_byte(abs(R) * MAX_COLOR)), MAX_COLOR)),
			g(std::min<png_byte>(std::max<png_byte>(0, png_byte(abs(G) * MAX_COLOR)), MAX_COLOR)),
			b(std::min<png_byte>(std::max<png_byte>(0, png_byte(abs(B) * MAX_COLOR)), MAX_COLOR)),
			a(std::min<png_byte>(std::max<png_byte>(0, png_byte(abs(A) * MAX_COLOR)), MAX_COLOR)),
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
			r = (png_byte)std::min<float>((float)MAX_COLOR, (color.float_r * float_r) * MAX_COLOR);
			g = (png_byte)std::min<float>((float)MAX_COLOR, (color.float_g * float_g) * MAX_COLOR);
			b = (png_byte)std::min<float>((float)MAX_COLOR, (color.float_b * float_b) * MAX_COLOR);
			a = (png_byte)std::min<float>((float)MAX_COLOR, (color.float_a * float_a) * MAX_COLOR);

			float_r *= color.float_r;
			float_g *= color.float_g;
			float_b *= color.float_b;
			float_a *= color.float_a;
		}

		Color operator*(const Color& color)
		{
			return Color(color.float_r * float_r, color.float_g * float_g, color.float_b * float_b, color.float_a * float_a);
		}

		Color operator*(float scalar) const
		{
			return Color(scalar * float_r, scalar * float_g, scalar * float_b, scalar * float_a);
		}

		Color operator+(const Color &color)
		{
			return Color(float_r + color.float_r, float_g + color.float_g, float_b + color.float_b, float_a + color.float_a);
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

