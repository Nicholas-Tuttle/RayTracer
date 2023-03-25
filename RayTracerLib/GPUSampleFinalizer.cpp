#include "GPUSampleFinalizer.h"
#include "VulkanUtils.h"

using RayTracer::GPUSampleFinalizer;
using RayTracer::VKUtils;
using RayTracer::PerformanceTracking::PerformanceSession;

const static size_t shader_local_size_x = 1024;

GPUSampleFinalizer::GPUSampleFinalizer(vk::Device device, uint32_t compute_queue_index, vk::Buffer input_gpu_sample_buffer, vk::Buffer output_gpu_color_buffer, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: SampleFinalizerPushConstants(), GPUComputeShader("GPUSampleFinalizer.comp.spv", compute_queue_index, 2, sizeof(GPUSampleFinalizer::SampleFinalizerPushConstants), device, std::vector<vk::Buffer>{input_gpu_sample_buffer, output_gpu_color_buffer}, session), performance_session(session)
{}

void GPUSampleFinalizer::WriteCommandBuffer(vk::CommandBuffer &buffer, size_t incoming_ray_count, uint32_t sample_count)
{
	TRACE_FUNCTION(performance_session);

	SampleFinalizerPushConstants.sample_count = sample_count;

	GPUComputeShader::WriteCommandBuffer(buffer, incoming_ray_count, static_cast<void *>(&SampleFinalizerPushConstants));
}

