#include "PrincipledBSDF.h"

using RayTracer::Color;
using RayTracer::Intersection;

const Color RayTracer::PrincipledBSDF::SurfaceColor() const
{
	return Color();
}

float RayTracer::PrincipledBSDF::Roughness() const
{
	return 0.0f;
}

void RayTracer::PrincipledBSDF::GetResultantRay(const Intersection &intersection, 
	const Ray &incoming_ray, Ray &outgoing_ray) const
{
	outgoing_ray = Ray();
}
