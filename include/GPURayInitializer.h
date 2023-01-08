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
		GPURayInitializer(vk::Device device);
		void Execute(uint32_t ComputeQueueIndex,
			Vector3<float> origin,
			Vector3<float> forward_vector,
			Vector3<float> right_vector,
			Vector3<float> up_vector,
			float focal_length_mm,
			float sensor_width_mm,
			size_t resolution_x,
			size_t resolution_y,
			size_t samples,
			size_t seed,
			size_t offset, 
			vk::Buffer output_ray_buffer);
	private:
		vk::DescriptorSetLayout DescribeShader();
		vk::Result CreatePipeline();
		std::vector<vk::DescriptorSet> AllocateDescriptorSets();
		void UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet, vk::Buffer output_ray_buffer);

		GPURayInitializer(const GPURayInitializer &other) = delete;
		GPURayInitializer(const GPURayInitializer &&other) = delete;

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
				samples = 1;
				seed = 0;
				offset = 0;
			}

			float camera_origin[4];
			float forward_vector[4];
			float right_vector[4];
			float up_vector[4];
			unsigned int focal_length_mm;
			unsigned int sensor_width_mm;
			unsigned int resolution_x;
			unsigned int resolution_y;
			unsigned int samples;
			unsigned int seed;
			unsigned int offset;
		} CameraDataPushConstants;
	};
}
