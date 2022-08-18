#pragma once

#include "Vector3.h"
#include "Color.h"

namespace RayTracer
{
	class Ray
	{
	private:
		Vector3<float> origin;
		Vector3<float> direction;
		Color ray_color;

	public:
		Ray(const Vector3<float> &origin, const Vector3<float> &direction, const Color &color) : 
			origin(origin), direction(direction), ray_color(color) {}
		Ray() :origin(), direction(), ray_color() {}
		Ray(const Ray &ray) : origin(ray.origin), direction(ray.direction) {}

		const Vector3<float> &Origin() const
		{
			return origin;
		}

		const Vector3<float> &Direction() const
		{
			return direction;
		}

		const Color &RayColor() const
		{
			return ray_color;
		}

		void SetDirection(const Vector3<float> &direction)
		{
			this->direction = direction;
		}

		void SetColor(const Color &color)
		{
			ray_color = color;
		}
	};
}