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
		bool emissive;
	public:
		Material(const Color &color, float roughness, bool emissive)
			: color(color), roughness(roughness), emissive(emissive) {}

		const Color SurfaceColor() const override
		{
			return color;
		}

		float Roughness() const override
		{
			return roughness;
		}

		bool Emissive() const override
		{
			return emissive;
		}

		static Material DefaultMaterial;
	};
}
