#pragma once

#include <Vectors.h>
#include <Color.h>

namespace RayTracer
{
	class IMaterial
	{
	public:
		virtual const Color SurfaceColor() const = 0;
		virtual float Roughness() const = 0;
		virtual bool Emissive() const = 0;
	};
}
