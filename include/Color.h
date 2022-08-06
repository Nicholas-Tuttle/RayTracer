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

	public:
		Color() : Color(MAX_COLOR, MAX_COLOR, MAX_COLOR, MAX_COLOR) {}
		Color(png_byte R, png_byte G, png_byte B, png_byte A)
			: r(R), g(G), b(B), a(A) {};

		Color(float R, float G, float B, float A)
			: 
				r(std::min<png_byte>(std::max<png_byte>(0, png_byte(abs(R) * MAX_COLOR)), MAX_COLOR)),
				g(std::min<png_byte>(std::max<png_byte>(0, png_byte(abs(G) * MAX_COLOR)), MAX_COLOR)),
				b(std::min<png_byte>(std::max<png_byte>(0, png_byte(abs(B) * MAX_COLOR)), MAX_COLOR)),
				a(std::min<png_byte>(std::max<png_byte>(0, png_byte(abs(A) * MAX_COLOR)), MAX_COLOR))
		{};

		void operator=(const Color &color)
		{
			r = color.r;
			g = color.g;
			b = color.b;
			a = color.a;
		}

		void operator*=(const Color &color)
		{
			r = (png_byte)((float)color.r * (float)r / MAX_COLOR);
			g = (png_byte)((float)color.g * (float)g / MAX_COLOR);
			b = (png_byte)((float)color.b * (float)b / MAX_COLOR);
			a = (png_byte)((float)color.a * (float)a / MAX_COLOR);
		}

		Color operator*(const Color& color)
		{
			return Color(
				(png_byte)((float)color.r * (float)r / MAX_COLOR), 
				(png_byte)((float)color.g * (float)g / MAX_COLOR), 
				(png_byte)((float)color.b * (float)b / MAX_COLOR), 
				(png_byte)((float)color.a * (float)a / MAX_COLOR)
			);
		}

		png_byte R() const { return r; }
		png_byte G() const { return g; }
		png_byte B() const { return b; }
		png_byte A() const { return a; }

		float R_float() const { return (float)r / MAX_COLOR; }
		float G_float() const { return (float)g / MAX_COLOR; }
		float B_float() const { return (float)b / MAX_COLOR; }
		float A_float() const { return (float)a / MAX_COLOR; }
	};
}

