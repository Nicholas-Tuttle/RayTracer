#pragma once

#include "IMaterial.h"
#include "Color.h"

namespace RayTracer
{
	class GlossyBSDF : public IMaterial
	{
	public:
		GlossyBSDF(const Color &color, float roughness) : color(color), roughness(roughness) {}
		// Inherited via IMaterial
		virtual const Color SurfaceColor() const override;
		virtual float Roughness() const override;
		virtual void GetResultantRay(std::unique_ptr<IIntersection> &intersection,
			const std::unique_ptr<IRay> &incoming_ray, std::unique_ptr<IRay> &outgoing_ray) const override;
	private:
		Color color;
		float roughness;
	};
}

