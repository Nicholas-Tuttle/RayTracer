#include "DiffuseBSDF.h"
#include "Ray.h"

using RayTracer::Color;

const Color RayTracer::DiffuseBSDF::SurfaceColor() const
{
	return color;
}

float RayTracer::DiffuseBSDF::Roughness() const
{
	return roughness;
}

void RayTracer::DiffuseBSDF::GetResultantRay(std::unique_ptr<IIntersection> &intersection,
	const std::unique_ptr<IRay> &incoming_ray, std::unique_ptr<IRay> &outgoing_ray) const
{
	Vector3<float> out_direction = intersection->Normal().Normalize() + (Vector3<float>::RandomInUnitSphere() * roughness);
	// TODO: figure out the modularity value
	outgoing_ray = std::make_unique<Ray>(intersection->Location(), out_direction, color.Modulate(incoming_ray->RayColor(), 0.2f));
}
