#pragma once

#include <IIntersectable.h>

namespace RayTracer
{
	class Ray : public IRay
	{
	private:
		Vector3<float> origin;
		Vector3<float> direction;

	public:
		Ray(const Vector3<float> &origin, const Vector3<float> &direction) : origin(origin), direction(direction) {}
		Ray() :origin(), direction() {}
		Ray(const Ray &ray) : origin(ray.origin), direction(ray.direction) {}

		const Vector3<float> &Origin() const override
		{
			return origin;
		}

		const Vector3<float> &Direction() const override
		{
			return direction;
		}

		void SetDirection(const Vector3<float> &direction)
		{
			this->direction = direction;
		}
	};
}