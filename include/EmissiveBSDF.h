#pragma once

#include "IMaterial.h"

namespace RayTracer
{
	class EmissiveBSDF : public IMaterial
	{
	public:
		EmissiveBSDF(const Color &color) : color(color) {}
		// Inherited via IMaterial
		virtual const Color SurfaceColor() const override;
		virtual float Roughness() const override;
		virtual void GetResultantRay(const Intersection &intersection,
			const Ray &incoming_ray, Ray &outgoing_ray) const override;
	private:
		Color color;
	};
}

