#pragma once

#include "GPUComputeShader.h"
#include <iostream>

namespace RayTracer
{
	class GPUSampleAccumulator : protected GPUComputeShader
	{
	public:
		GPUSampleAccumulator(vk::Device device);
		void Execute(uint32_t compute_queue_index,
			size_t incoming_ray_count,
			vk::Buffer input_gpu_intersection_buffer,
			vk::Buffer output_gpu_color_buffer,
			bool finalize = false,
			uint32_t sample_count = 0);
	private:
		vk::DescriptorSetLayout DescribeShader();
		vk::Result CreatePipeline();
		std::vector<vk::DescriptorSet> AllocateDescriptorSets();
		void UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet, 
			vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_ray_buffer);

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
	};
}
