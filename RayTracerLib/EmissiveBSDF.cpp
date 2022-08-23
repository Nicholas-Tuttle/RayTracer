#include "EmissiveBSDF.h"

using RayTracer::Color;
using RayTracer::Ray;
using RayTracer::Intersection;

const Color RayTracer::EmissiveBSDF::SurfaceColor() const
{
	return color;
}

float RayTracer::EmissiveBSDF::Roughness() const
{
	return 0.0f;
}

void RayTracer::EmissiveBSDF::GetResultantRay(const Intersection &intersection,
	const Ray &incoming_ray, Ray &outgoing_ray) const
{
	outgoing_ray = Ray(incoming_ray.Origin(), Vector3<float>(0, 0, 0), Color(incoming_ray.RayColor() * color * strength));
}
