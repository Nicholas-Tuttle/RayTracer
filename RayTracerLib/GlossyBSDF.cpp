#include "GlossyBSDF.h"

using RayTracer::Color;
using RayTracer::Vector3;
using RayTracer::Intersection;

const Color RayTracer::GlossyBSDF::SurfaceColor() const
{
	return color;
}

float RayTracer::GlossyBSDF::Roughness() const
{
	return roughness;
}

void RayTracer::GlossyBSDF::GetResultantRay(const Intersection &intersection,
	const Ray &incoming_ray, Ray &outgoing_ray) const
{
	Vector3<float> in_direction = incoming_ray.Direction().Normalize();
	Vector3<float> intersection_normal = intersection.Normal().Normalize();

	Vector3<float> out_reflection = in_direction - intersection_normal * 2 * in_direction.Dot(intersection_normal);
	Vector3<float> out_reflection_offset = out_reflection + (Vector3<float>::RandomInUnitSphere() * roughness);

	if (out_reflection_offset.Dot(intersection_normal) <= 0)
	{
		outgoing_ray = Ray(intersection.Location(), out_reflection, Color(incoming_ray.RayColor() * color));
	}
	else
	{
		outgoing_ray = Ray(intersection.Location(), out_reflection_offset, Color(incoming_ray.RayColor() * color));
	}
}
