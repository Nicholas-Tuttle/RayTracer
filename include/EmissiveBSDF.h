#pragma once

#include "IMaterial.h"
#include "Ray.h"

namespace RayTracer
{
	class EmissiveBSDF : public IMaterial
	{
	public:
		EmissiveBSDF(const Color &color) : color(color) {}
		// Inherited via IMaterial
		virtual const Color SurfaceColor() const override;
		virtual float Roughness() const override;
		virtual void GetResultantRay(std::unique_ptr<IIntersection> &intersection,
			const std::unique_ptr<IRay> &incoming_ray, std::unique_ptr<IRay> &outgoing_ray) const override;
	private:
		Color color;
	};
}

