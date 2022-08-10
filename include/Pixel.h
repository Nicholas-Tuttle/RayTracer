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
		size_t accumulatedSamples;
		size_t xCoordinate;
		size_t yCoordinate;
		float widthM;
		float heightM;

	public:
		Pixel(const Vector3<float> &origin, const Vector3<float> &centralRayDirection, size_t x, size_t y, float widthM, float heightM)
			: output(), origin(origin), centralRayDirection(centralRayDirection), accumulatedSamples(0),
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
			accumulatedSamples = 1;
			output = color;
		};

		void AccumulateColorSample(const Color& color)
		{
			float r_accumulated = (float)output.R() * accumulatedSamples;
			float g_accumulated = (float)output.G() * accumulatedSamples;
			float b_accumulated = (float)output.B() * accumulatedSamples;
			float a_accumulated = (float)output.A() * accumulatedSamples;

			r_accumulated += color.R();
			g_accumulated += color.G();
			b_accumulated += color.B();
			a_accumulated += color.A();

			accumulatedSamples++;

			Color accumulated_color(
				(png_byte)(r_accumulated / accumulatedSamples), 
				(png_byte)(g_accumulated / accumulatedSamples),
				(png_byte)(b_accumulated / accumulatedSamples),
				(png_byte)(a_accumulated / accumulatedSamples));

			output = accumulated_color;
		}

		const Vector3<float> &CentralRayDirection()
		{
			return centralRayDirection;
		}

		std::unique_ptr<IRay> GetNextRay()
		{
			//Get 2 random floats -0.5 <= float <= 0.5
			float random1 = (float)(rand() - (RAND_MAX / 2)) / RAND_MAX;
			float random2 = (float)(rand() - (RAND_MAX / 2)) / RAND_MAX;

			Vector3<float> jitteredRayDirection = centralRayDirection + (rightVector * random1 * widthM) + (upVector * random2 * heightM);

			return std::make_unique<Ray>(origin, jitteredRayDirection);
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
