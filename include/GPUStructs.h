#pragma once

#include "Sphere.h"

namespace RayTracer
{
	// When editing the structs in the shader code calculate the size 
	// of the GPU struct and include alignment in these structs 

	struct GPURay
	{
		float origin[4];
		float direction[4];
		uint32_t pixelXIndex;
		uint32_t pixelYIndex;
		float padding[2]; // Needed for "vec4" alignment
	};

	struct GPUIntersection
	{
		float ray_origin[4];
		float ray_direction[4];
		float intersection_location[4];
		float intersection_normal[4];
		float ray_color[4];
		float depth;
		uint32_t object_id;
		uint32_t face_id;
		uint32_t material_id;
		uint32_t pixelXIndex;
		uint32_t pixelYIndex;
		float padding[2]; // Needed for "vec4" alignment
	};

	struct GPUSphere
	{
		GPUSphere(const Sphere &sphere)
		{
			const Vector3<float> &sphere_position = sphere.Position();
			position[0] = sphere_position.X;
			position[1] = sphere_position.Y;
			position[2] = sphere_position.Z;
			position[3] = 0;

			radius = sphere.Radius();
			material_index = sphere.MaterialIndex();
		}

		GPUSphere(const Sphere *sphere)
		{
			if (nullptr == sphere)
			{
				return;
			}

			const Vector3<float> &sphere_position = sphere->Position();
			position[0] = sphere_position.X;
			position[1] = sphere_position.Y;
			position[2] = sphere_position.Z;
			position[3] = 0;

			radius = sphere->Radius();
			material_index = sphere->MaterialIndex();
		}

		float position[4];
		float radius;
		uint32_t material_index;
		float padding[2]; // Needed for "vec4" alignment
	};
}
