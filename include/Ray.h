#pragma once

#include <IIntersectable.h>

namespace RayTracer
{
	class Ray : public IRay
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

		const Vector3<float> &Origin() const override
		{
			return origin;
		}

		const Vector3<float> &Direction() const override
		{
			return direction;
		}

		Color &RayColor() override
		{
			return ray_color;
		}

		void SetDirection(const Vector3<float> &direction)
		{
			this->direction = direction;
		}
	};
}