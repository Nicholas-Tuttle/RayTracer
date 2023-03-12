#pragma once

#include "GPUComputeShader.h"
#include "Camera.h"
#include "GPUStructs.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPURayInitializer : protected GPUComputeShader
	{
	public:
		GPURayInitializer(vk::Device device, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
		void Execute(uint32_t ComputeQueueIndex, Camera camera, size_t seed, vk::Buffer output_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer);
	private:
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

		const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
	};
}
