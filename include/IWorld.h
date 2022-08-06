#pragma once

#include "Color.h"
#include "Vectors.h"

namespace RayTracer
{
	class IWorld
	{
	public:
		virtual Color SurfaceColor(Vector3<float> normal) const = 0;
		virtual Color AmbientColor() const = 0;
	};
}
