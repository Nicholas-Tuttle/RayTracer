#pragma once

#include "GPUComputeShader.h"
#include <vulkan/vulkan.hpp>
#include "GPUStructs.h"
#include <iostream>

namespace RayTracer
{
	class GPUDiffuseMaterial : protected GPUComputeShader
	{
	public:
		GPUDiffuseMaterial(vk::Device device);
		void Execute(uint32_t ComputeQueueIndex, size_t incoming_ray_count,
			vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer);
	private:
		vk::DescriptorSetLayout DescribeShader();
		vk::Result CreatePipeline();
		std::vector<vk::DescriptorSet> AllocateDescriptorSets();
		void UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet,
			vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer);

		GPUDiffuseMaterial(const GPUDiffuseMaterial &other) = delete;
		GPUDiffuseMaterial(const GPUDiffuseMaterial &&other) = delete;

		struct push_constants
		{
			push_constants()
			{
				material_id = 0;
			}

			unsigned int material_id;
		} DiffuseMaterialPushConstants;
	};
}
