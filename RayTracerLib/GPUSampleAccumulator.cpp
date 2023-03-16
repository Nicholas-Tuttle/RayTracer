#include "GPUSampleAccumulator.h"
#include "VulkanUtils.h"

using RayTracer::GPUSampleAccumulator;
using RayTracer::VKUtils;
using RayTracer::PerformanceTracking::PerformanceSession;

const static size_t shader_local_size_x = 1024;

GPUSampleAccumulator::GPUSampleAccumulator(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: GPUComputeShader("GPUSampleAccumulator.comp.spv", compute_queue_index, 2, 0, device, session), performance_session(session)
{}

void GPUSampleAccumulator::Execute(size_t incoming_ray_count, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_sample_buffer)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::Execute(incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_sample_buffer});
}

