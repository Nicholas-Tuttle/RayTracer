#pragma once

#include "GPUComputeShader.h"

#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPUSampleFinalizer : public GPUComputeShader
	{
	public:
		GPUSampleFinalizer(vk::Device device, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
		void Execute(uint32_t compute_queue_index,
			size_t incoming_ray_count,
			vk::Buffer input_gpu_intersection_buffer,
			vk::Buffer output_gpu_color_buffer,
			uint32_t sample_count = 0);
	private:
		GPUSampleFinalizer(const GPUSampleFinalizer &other) = delete;
		GPUSampleFinalizer(const GPUSampleFinalizer &&other) = delete;

		struct push_constants
		{
			push_constants()
			{
				sample_count = 0;
			}

			uint32_t sample_count;
		} SampleFinalizerPushConstants;

		const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
	};
}
