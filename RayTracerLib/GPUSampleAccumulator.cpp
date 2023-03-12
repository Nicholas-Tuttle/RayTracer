#include "GPUSampleAccumulator.h"
#include "VulkanUtils.h"

using RayTracer::GPUSampleAccumulator;
using RayTracer::VKUtils;
using RayTracer::PerformanceTracking::PerformanceSession;

const static size_t shader_local_size_x = 1024;

GPUSampleAccumulator::GPUSampleAccumulator(vk::Device device, PerformanceSession *const session)
	: SampleAccumulatorPushConstants(), GPUComputeShader("GPUSampleAccumulator.comp.spv", 2, sizeof(GPUSampleAccumulator::SampleAccumulatorPushConstants), device, session), performance_session(session)
{}

void GPUSampleAccumulator::Execute(uint32_t compute_queue_index,
	size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer,
	vk::Buffer output_gpu_sample_buffer,
	bool finalize,
	uint32_t sample_count)
{
	TRACE_FUNCTION(performance_session);

	SampleAccumulatorPushConstants.finalize = finalize;
	SampleAccumulatorPushConstants.sample_count = sample_count;

	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_sample_buffer}, static_cast<void *>(&SampleAccumulatorPushConstants));
}

