#pragma once

namespace RayTracer
{
	struct GPURay
	{
		float direction[4];
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
	};
}
