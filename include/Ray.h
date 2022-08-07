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

		static Ray ReflectionRay(const Ray &incoming_ray, const IIntersection *intersection)
		{
			/*
			  
			  ___ ___ a
			  \  |  /
			  r\ |b/ incoming_ray
			    \|/
			  
			*/

			Vector3<float> intersection_normal = intersection->Normal();
			//float intersection_roughness = intersection->Roughness();
			float intersection_roughness = 0.05f;

			Vector3<float> b = intersection_normal * incoming_ray.direction.Dot(intersection_normal);
			Vector3<float> a = incoming_ray.direction - b;
			Vector3<float> r = ((b * -1) + a).Normalize();

			// TODO: This isn't lambertian reflection at all, probably should be
			if (intersection_roughness > 0)
			{
				// Get random values between -1.0 and 1.0
				const float half_rand_max = RAND_MAX / 2;
				float random1 = ((float)rand() - half_rand_max) / half_rand_max;
				float random2 = ((float)rand() - half_rand_max) / half_rand_max;
				float random3 = ((float)rand() - half_rand_max) / half_rand_max;

				Vector3<float> jittered_refraction_ray = Vector3<float>(
					random1 + intersection_normal.X,
					random2 + intersection_normal.Y,
					random3 + intersection_normal.Z
				).Normalize();

				return Ray(intersection->Location(), r.NormalizedLerp(jittered_refraction_ray, intersection_roughness));
			}

			return Ray(intersection->Location(), r);
		}
	};
}