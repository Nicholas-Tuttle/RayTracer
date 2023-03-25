#pragma once

#include "GPUComputeShader.h"
#include <iostream>

#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPUSampleAccumulator : protected GPUComputeShader
	{
	public:
		GPUSampleAccumulator(vk::Device device, uint32_t compute_queue_index, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_color_buffer, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
		void WriteCommandBuffer(std::vector<std::reference_wrapper<vk::CommandBuffer>> buffers, size_t incoming_ray_count);

		uint32_t RequiredCommandBuffers() const
		{
			return 2;
		}
	private:
		vk::Device Device;

		GPUSampleAccumulator(const GPUSampleAccumulator &other) = delete;
		GPUSampleAccumulator(const GPUSampleAccumulator &&other) = delete;

		const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
	};
}
