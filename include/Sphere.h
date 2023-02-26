#pragma once

#include <Vector3.h>
#include <IIntersectable.h>
#include <Intersection.h>
#include "DiffuseBSDF.h"

namespace RayTracer
{
	class Sphere : public IIntersectable
	{
	private:
		Vector3<float> position;
		float radius;
		const std::shared_ptr<const IMaterial> material;
	public:
		Sphere(const Vector3<float> &position, const float radius, const std::shared_ptr<const IMaterial> material)
			: position(position), radius(radius), material(material) {}
		Sphere(const Vector3<float> &position, const float radius)
			: position(position), radius(radius), material(std::make_shared<const DiffuseBSDF>(Color(), 1.0f)) {}

		Vector3<float> Position() const
		{
			return position;
		}

		float Radius() const
		{
			return radius;
		}

		const std::shared_ptr<const IMaterial> Material() const override
		{
			return material;
		}

		bool IntersectsRay(const Ray &incoming_ray, Intersection &out_intersection_info) const override
		{
			Vector3<float> line_origin_to_sphere_center_O_minus_C(position - incoming_ray.Origin());
			float u_dot = line_origin_to_sphere_center_O_minus_C.Dot(incoming_ray.Direction().Normalize());

			float dot_squared = u_dot * u_dot;
			float magnitude_squared = line_origin_to_sphere_center_O_minus_C.MagnitudeSquared();
			float radius_squared = radius * radius;

			float delta = dot_squared + radius_squared - magnitude_squared;

			if (delta < 0)
			{
				return false;
			}

			// The closest depth has to be the "minus" term of the "plus or minus" square-root since delta is always positive
			float depth = u_dot - sqrt(delta);

			// If the depth is negative either only the backwards ray intersects, or it is a backface from starting in the sphere
			if (depth < 0)
			{
				return false;
			}

			Vector3<float> intersection_location = incoming_ray.Origin() + incoming_ray.Direction().Normalize() * depth;
			Vector3<float> intersection_normal = (intersection_location - position).Normalize();

			out_intersection_info = Intersection(depth, intersection_normal, intersection_location);

			return true;
		}
	};
}