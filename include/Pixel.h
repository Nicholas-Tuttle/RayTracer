#pragma once

#include "Color.h"
#include "Vector3.h"
#include "IIntersectable.h"
#include "Ray.h"
#include <vector>

namespace RayTracer
{
	class Pixel
	{
	private:
		Color output;
		Vector3<float> origin;
		Vector3<float> centralRayDirection;
		Vector3<float> rightVector;
		Vector3<float> upVector;
		size_t xCoordinate;
		size_t yCoordinate;
		float widthM;
		float heightM;

	public:
		Pixel(const Vector3<float> &origin, const Vector3<float> &centralRayDirection, size_t x, size_t y, float widthM, float heightM)
			: output(), origin(origin), centralRayDirection(centralRayDirection), 
			xCoordinate(x), yCoordinate(y), widthM(widthM), heightM(heightM)
		{
			if (centralRayDirection == Vector3<float>(0, 1, 0))
			{
				rightVector = Vector3<float>(1, 0, 0);
				upVector = Vector3<float>(0, 1, 0);
			}
			else
			{
				rightVector = centralRayDirection.Cross(Vector3<float>(0, 1, 0)).Normalize();
				upVector = rightVector.Cross(centralRayDirection).Normalize();
			}
		}

		Color OutputColor() const
		{
			return output;
		}

		void SetColor(const Color &color)
		{
			output = color;
		};

		void Average(const std::vector<Color> &colors)
		{
			float r = 0;
			float g = 0;
			float b = 0;
			float a = 0;

			for (const auto &color : colors)
			{
				r += color.R_float();
				g += color.G_float();
				b += color.B_float();
				a += color.A_float();
			}

			size_t samples = colors.size();

			png_byte final_r = (png_byte)(std::min<float>(1.0f, (r / samples)) * Color::MAX_COLOR);
			png_byte final_g = (png_byte)(std::min<float>(1.0f, (g / samples)) * Color::MAX_COLOR);
			png_byte final_b = (png_byte)(std::min<float>(1.0f, (b / samples)) * Color::MAX_COLOR);
			png_byte final_a = (png_byte)(std::min<float>(1.0f, (a / samples)) * Color::MAX_COLOR);

			output = Color(final_r, final_g, final_b, final_a);
		}

		const Vector3<float> &CentralRayDirection()
		{
			return centralRayDirection;
		}

		Ray GetNextRay()
		{
			//Get 2 random floats -0.5 <= float <= 0.5
			float random1 = (float)(rand() - (RAND_MAX / 2)) / RAND_MAX;
			float random2 = (float)(rand() - (RAND_MAX / 2)) / RAND_MAX;

			Vector3<float> jitteredRayDirection = centralRayDirection + (rightVector * random1 * widthM) + (upVector * random2 * heightM);

			return Ray(origin, jitteredRayDirection, Color());
		}

		size_t XCoordinate() const
		{
			return xCoordinate;
		}

		size_t YCoordinate() const
		{
			return yCoordinate;
		}
	};
}

#pragma once
