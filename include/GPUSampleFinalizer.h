#pragma once

#include "GPUComputeShader.h"

#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPUSampleFinalizer : public GPUComputeShader
	{
	public:
		GPUSampleFinalizer(vk::Device device, uint32_t compute_queue_index, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_color_buffer, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
		void WriteCommandBuffer(vk::CommandBuffer &buffer, size_t incoming_ray_count, uint32_t sample_count);

		uint32_t RequiredCommandBuffers() const
		{
			return 1;
		}
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
