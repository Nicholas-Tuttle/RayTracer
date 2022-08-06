#pragma once

#include "Color.h"
#include "IMaterial.h"
#include "Vectors.h"

namespace RayTracer
{
	class Material : public IMaterial
	{
	private:
		Color color;
		float roughness;
	public:
		Material(const Color &color, const float roughness)
			: color(color), roughness(roughness) {}

		const Color SurfaceColor() const override
		{
			return color;
		}

		const float Roughness() const override
		{
			return roughness;
		}

		static Material DefaultMaterial;
	};
}
