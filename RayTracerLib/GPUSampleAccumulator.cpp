#include "GPUSampleAccumulator.h"
#include "VulkanUtils.h"

using RayTracer::GPUSampleAccumulator;
using RayTracer::VKUtils;

const static size_t shader_local_size_x = 1024;

GPUSampleAccumulator::GPUSampleAccumulator(vk::Device device)
	: SampleAccumulatorPushConstants(), GPUComputeShader("GPUSampleAccumulator.comp.spv", 2, sizeof(GPUSampleAccumulator::SampleAccumulatorPushConstants), device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << std::endl;
#endif
}

void GPUSampleAccumulator::Execute(uint32_t compute_queue_index,
	size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer,
	vk::Buffer output_gpu_sample_buffer,
	bool finalize,
	uint32_t sample_count)
{
	SampleAccumulatorPushConstants.finalize = finalize;
	SampleAccumulatorPushConstants.sample_count = sample_count;

	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_sample_buffer}, static_cast<void *>(&SampleAccumulatorPushConstants));
}

