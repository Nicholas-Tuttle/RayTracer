#pragma once

#include "IMaterial.h"
#include "Intersection.h"

namespace RayTracer
{
	class PrincipledBSDF : public IMaterial
	{
	public:
		// Inherited via IMaterial
		virtual const Color SurfaceColor() const override;
		virtual float Roughness() const override;
		virtual void GetResultantRay(const Intersection &intersection, 
			const Ray &incoming_ray, Ray &outgoing_ray) const override;
	};
}

