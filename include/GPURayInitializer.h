#pragma once

#include "GPUComputeShader.h"
#include "Camera.h"
#include "GPUStructs.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

namespace RayTracer
{
	class GPURayInitializer : protected GPUComputeShader
	{
	public:
		GPURayInitializer(vk::Device device, const Camera &camera, size_t resolution_x, size_t resolution_y,
			GPURay *gpu_ray_buffer, vk::Buffer output_ray_buffer);
		void Execute(uint32_t ComputeQueueIndex);
	private:
		vk::DescriptorSetLayout DescribeShader();
		vk::Result CreatePipeline();
		void AllocateAndUpdateDescriptorSets();
		std::vector<vk::DescriptorSet> AllocateDescriptorSets();
		void UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet);

		GPURayInitializer(const GPURayInitializer &other) = delete;
		GPURayInitializer(const GPURayInitializer &&other) = delete;

		size_t ResolutionX;
		size_t ResolutionY;

		vk::Buffer OutputRayBuffer = nullptr;
		GPURay *gpu_ray_buffer = nullptr;

		struct push_constants
		{
			push_constants()
			{
				memset(camera_origin, 0, sizeof(camera_origin));
				memset(forward_vector, 0, sizeof(forward_vector));
				memset(right_vector, 0, sizeof(right_vector));
				memset(up_vector, 0, sizeof(up_vector));
				focal_length_mm = 0;
				sensor_width_mm = 0;
				resolution_x = 0;
				resolution_y = 0;
				seed = 0;
			}

			float camera_origin[4];
			float forward_vector[4];
			float right_vector[4];
			float up_vector[4];
			unsigned int focal_length_mm;
			unsigned int sensor_width_mm;
			unsigned int resolution_x;
			unsigned int resolution_y;
			unsigned int seed;
		} CameraDataPushConstants;
	};
}
