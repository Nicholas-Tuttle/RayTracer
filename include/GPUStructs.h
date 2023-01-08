#pragma once

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
}
