#pragma once

#include "GPUComputeShader.h"
#include <vulkan/vulkan.hpp>
#include "GPUStructs.h"
#include <iostream>
#include "GPURenderer.h"

namespace RayTracer
{
	class GPUDiffuseMaterial : protected GPUComputeShader
	{
	public:
		GPUDiffuseMaterial(vk::Device device);
		void Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
			vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, 
			vk::Buffer input_gpu_material_parameters);
	private:
		GPUDiffuseMaterial(const GPUDiffuseMaterial &other) = delete;
		GPUDiffuseMaterial(const GPUDiffuseMaterial &&other) = delete;

		struct push_constants
		{
			push_constants()
			{
				material_id = static_cast<uint32_t>(RayTracer::GPURenderer::MaterialTypeID::diffuse);
			}

			uint32_t material_id;
		} DiffuseMaterialPushConstants;
	};
}

