#pragma once

#include "Vector3.h"
#include "Color.h"
#include "IIntersectable.h"

namespace RayTracer
{
	class IMaterial
	{
	public:
		virtual const Color SurfaceColor() const = 0; // TODO: This should be part of GetResultantRay
		virtual float Roughness() const = 0; // TODO: This is not applicable to all materials
		virtual void GetResultantRay(std::unique_ptr<IIntersection> &intersection, const std::unique_ptr<IRay> &incoming_ray,
			std::unique_ptr<IRay> &outgoing_ray) const = 0;
	};
}
