#pragma once

#include "Vector3.h"
#include "Color.h"
#include "Ray.h"
#include "Intersection.h"

namespace RayTracer
{
	class IIntersectable
	{
	public:
		virtual bool IntersectsRay(const Ray &incoming_ray, Intersection &out_intersection_info) const = 0;
		virtual int MaterialIndex() const = 0;
	};
}
