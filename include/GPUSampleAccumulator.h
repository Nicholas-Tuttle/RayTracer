#pragma once

#include "GPUComputeShader.h"
#include <iostream>

#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPUSampleAccumulator : protected GPUComputeShader
	{
	public:
		GPUSampleAccumulator(vk::Device device, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
		void Execute(uint32_t compute_queue_index,
			size_t incoming_ray_count,
			vk::Buffer input_gpu_intersection_buffer,
			vk::Buffer output_gpu_color_buffer);
	private:
		GPUSampleAccumulator(const GPUSampleAccumulator &other) = delete;
		GPUSampleAccumulator(const GPUSampleAccumulator &&other) = delete;

		const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
	};
}
