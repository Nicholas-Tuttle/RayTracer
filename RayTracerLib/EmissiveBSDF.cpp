#include "EmissiveBSDF.h"

using RayTracer::Color;
using RayTracer::Ray;

const Color RayTracer::EmissiveBSDF::SurfaceColor() const
{
	return color;
}

float RayTracer::EmissiveBSDF::Roughness() const
{
	return 0.0f;
}

void RayTracer::EmissiveBSDF::GetResultantRay(std::unique_ptr<IIntersection> &intersection,
	const std::unique_ptr<IRay> &incoming_ray, std::unique_ptr<IRay> &outgoing_ray) const
{
	outgoing_ray = std::make_unique<Ray>(incoming_ray->Origin(), Vector3<float>(0, 0, 0), Color(incoming_ray->RayColor() * color));
}
