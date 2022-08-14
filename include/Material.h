#pragma once

#include "Color.h"
#include "IMaterial.h"
#include "Vector3.h"
#include "Ray.h"

namespace RayTracer
{
	// TODO: Split this class into multiple classes, and probably delete this file
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

		virtual void GetResultantRay(std::unique_ptr<IIntersection> &intersection, const std::unique_ptr<IRay> &incoming_ray_to_replace,
			std::unique_ptr<IRay> &outgoing_ray) const override
		{
			if (emissive)
			{
				outgoing_ray = nullptr;
				return;
			}

			/*

			  ___ ___ a
			  \  |  /
			  r\ |b/ incoming_ray
				\|/

			*/

			Vector3<float> intersection_normal = intersection->Normal();
			float intersection_roughness = 0.05f;

			Vector3<float> b = intersection_normal * incoming_ray_to_replace->Direction().Dot(intersection_normal);
			Vector3<float> a = incoming_ray_to_replace->Direction() - b;
			Vector3<float> r = ((b * -1) + a).Normalize();

			// TODO: This isn't lambertian reflection at all, probably should be
			if (intersection_roughness > 0)
			{
				// Get random values between -1.0 and 1.0
				const float half_rand_max = RAND_MAX / 2;
				float random1 = ((float)rand() - half_rand_max) / half_rand_max;
				float random2 = ((float)rand() - half_rand_max) / half_rand_max;
				float random3 = ((float)rand() - half_rand_max) / half_rand_max;

				Vector3<float> jittered_refraction_ray = Vector3<float>(
					random1 + intersection_normal.X,
					random2 + intersection_normal.Y,
					random3 + intersection_normal.Z
					).Normalize();

				outgoing_ray = std::unique_ptr<IRay>(new Ray(intersection->Location(), r.NormalizedLerp(jittered_refraction_ray, intersection_roughness),
					Color(incoming_ray_to_replace->RayColor() * color)));
				return;
			}

			outgoing_ray = std::unique_ptr<IRay>(new Ray(intersection->Location(), r, Color(incoming_ray_to_replace->RayColor() * color)));
		}

		static Material DefaultMaterial;
	};
}
