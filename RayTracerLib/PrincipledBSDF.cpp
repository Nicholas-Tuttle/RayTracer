#include "PrincipledBSDF.h"

using RayTracer::Color;

const Color RayTracer::PrincipledBSDF::SurfaceColor() const
{
	return Color();
}

float RayTracer::PrincipledBSDF::Roughness() const
{
	return 0.0f;
}

void RayTracer::PrincipledBSDF::GetResultantRay(std::unique_ptr<IIntersection> &intersection, 
	const std::unique_ptr<IRay> &incoming_ray, std::unique_ptr<IRay> &outgoing_ray) const
{
	outgoing_ray = nullptr;
}
