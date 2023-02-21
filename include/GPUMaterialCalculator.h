#pragma once

#include <vulkan/vulkan.hpp>
#include <iostream>
#include "GPUWorldMaterial.h"
#include "GPUDiffuseMaterial.h"

namespace RayTracer
{
	class GPUMaterialCalculator
	{
	public:
		GPUMaterialCalculator(vk::Device device);
		void Execute(uint32_t compute_queue_index,
			size_t incoming_ray_count,
			vk::Buffer input_gpu_intersection_buffer,
			vk::Buffer output_gpu_ray_buffer,
			vk::Buffer input_diffuse_material_parameters);

	private:
		vk::Device device;

		GPUDiffuseMaterial diffuse_material;
		GPUWorldMaterial world_material;
	};
}
