#pragma once

#include "Color.h"
#include "Vector3.h"

namespace RayTracer
{
	class IWorld
	{
	public:
		virtual Color SurfaceColor(const Vector3<float> &normal) const = 0;
		virtual const Color &AmbientColor() const = 0;
	};
}
