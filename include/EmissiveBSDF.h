#pragma once

#include "IMaterial.h"

namespace RayTracer
{
	class EmissiveBSDF : public IMaterial
	{
	public:
		EmissiveBSDF(const Color &color) : color(color), strength(1.0) {}
		EmissiveBSDF(const Color &color, float strength) : color(color), strength(strength) {}
		// Inherited via IMaterial
		virtual const Color SurfaceColor() const override;
		virtual float Roughness() const override;
		virtual void GetResultantRay(const Intersection &intersection,
			const Ray &incoming_ray, Ray &outgoing_ray) const override;
		float Strength() const { return strength; }
	private:
		Color color;
		float strength;
	};
}

