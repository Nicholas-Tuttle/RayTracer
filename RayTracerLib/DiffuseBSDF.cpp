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

void RayTracer::DiffuseBSDF::GetResultantRay(const Intersection &intersection,
	const Ray &incoming_ray, Ray &outgoing_ray) const
{
	Vector3<float> out_direction = intersection.Normal().Normalize() + (Vector3<float>::RandomInUnitSphere() * roughness);
	// TODO: figure out the modularity value
	outgoing_ray = Ray(intersection.Location(), out_direction, color.Modulate(incoming_ray.RayColor(), 0.2f));
}
