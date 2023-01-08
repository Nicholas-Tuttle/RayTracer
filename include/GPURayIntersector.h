#pragma once

#include "GPUComputeShader.h"
#include "GPUStructs.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

namespace RayTracer
{
	class GPURayIntersector : protected GPUComputeShader
	{
	public:
		GPURayIntersector(vk::Device device);
		void Execute(uint32_t ComputeQueueIndex, 
			size_t incoming_ray_count,
			vk::Buffer input_gpu_ray_buffer, 
			vk::Buffer output_gpu_intersection_buffer);
	private:
		vk::DescriptorSetLayout DescribeShader();
		vk::Result CreatePipeline();
		std::vector<vk::DescriptorSet> AllocateDescriptorSets();
		void UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet,
			vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer);

		GPURayIntersector(const GPURayIntersector &other) = delete;
		GPURayIntersector(const GPURayIntersector &&other) = delete;
	};
}
