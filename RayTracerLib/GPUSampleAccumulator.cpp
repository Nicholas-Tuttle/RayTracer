#include "GPUSampleAccumulator.h"
#include "VulkanUtils.h"

using RayTracer::GPUSampleAccumulator;
using RayTracer::VKUtils;
using RayTracer::PerformanceTracking::PerformanceSession;

const static size_t shader_local_size_x = 1024;

GPUSampleAccumulator::GPUSampleAccumulator(vk::Device device, uint32_t compute_queue_index, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_sample_buffer, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: Device(device), GPUComputeShader("GPUSampleAccumulator.comp.spv", compute_queue_index, 2, 0, device, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_sample_buffer}, session), performance_session(session)
{}

void GPUSampleAccumulator::WriteCommandBuffer(std::vector<std::reference_wrapper<vk::CommandBuffer>> buffers, size_t incoming_ray_count)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::WriteCommandBuffer(buffers[0].get(), incoming_ray_count);

	GPUComputeShader::WriteCalculationCompleteEvent(Device, buffers[buffers.size() - 1].get());
}

