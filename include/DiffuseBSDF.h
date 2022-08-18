#pragma once

#include "IMaterial.h"

namespace RayTracer
{
	class DiffuseBSDF : public IMaterial
	{
	public:
		DiffuseBSDF(const Color &color, float roughness) : color(color), roughness(roughness) {}
		// Inherited via IMaterial
		virtual const Color SurfaceColor() const override;
		virtual float Roughness() const override;
		virtual void GetResultantRay(const Intersection &intersection,
			const Ray &incoming_ray, Ray &outgoing_ray) const override;
	private:
		Color color;
		float roughness;
	};
}

