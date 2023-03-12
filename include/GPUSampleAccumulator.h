#pragma once

#include "GPUComputeShader.h"
#include <iostream>

#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPUSampleAccumulator : protected GPUComputeShader
	{
	public:
		GPUSampleAccumulator(vk::Device device, PerformanceTracking::PerformanceSession *const session);
		void Execute(uint32_t compute_queue_index,
			size_t incoming_ray_count,
			vk::Buffer input_gpu_intersection_buffer,
			vk::Buffer output_gpu_color_buffer,
			bool finalize = false,
			uint32_t sample_count = 0);
	private:
		GPUSampleAccumulator(const GPUSampleAccumulator &other) = delete;
		GPUSampleAccumulator(const GPUSampleAccumulator &&other) = delete;

		struct push_constants
		{
			push_constants()
			{
				sample_count = 0;
				finalize = false;
			}

			uint32_t sample_count;
			bool finalize;
		} SampleAccumulatorPushConstants;

		PerformanceTracking::PerformanceSession *performance_session;
	};
}
