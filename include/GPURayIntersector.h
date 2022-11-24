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
		GPURayIntersector(vk::Device device, size_t incoming_ray_count, GPURay *gpu_ray_buffer, 
			GPUIntersection *gpu_intersection_buffer, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_ray_buffer);
		void Execute(uint32_t ComputeQueueIndex);
	private:
		vk::DescriptorSetLayout DescribeShader();
		vk::Result CreatePipeline();
		void AllocateAndUpdateDescriptorSets();
		std::vector<vk::DescriptorSet> AllocateDescriptorSets();
		void UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet);

		GPURayIntersector(const GPURayIntersector &other) = delete;
		GPURayIntersector(const GPURayIntersector &&other) = delete;

		size_t IncomingRayBufferCount;

		vk::Buffer InputRayBuffer = nullptr;
		GPURay *gpu_ray_buffer = nullptr;
		vk::Buffer OutputIntersectionBuffer = nullptr;
		GPUIntersection *gpu_intersection_buffer = nullptr;
		// TODO: Add scene data
	};
}
