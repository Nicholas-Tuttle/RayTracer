#pragma once

#include "Vector3.h"
#include "Color.h"
#include "Intersection.h"
#include "Ray.h"

namespace RayTracer
{
	class IMaterial
	{
	public:
		virtual const Color SurfaceColor() const = 0; // TODO: This should be part of GetResultantRay
		virtual float Roughness() const = 0; // TODO: This is not applicable to all materials
		virtual void GetResultantRay(const Intersection &intersection, const Ray &incoming_ray, Ray &outgoing_ray) const = 0;
	};
}
