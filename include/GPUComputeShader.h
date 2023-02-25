#pragma once

#include <vulkan/vulkan.hpp>

namespace RayTracer
{
	class GPUComputeShader
	{
	protected:
		GPUComputeShader(const std::string &shader_file_name, size_t buffer_count, size_t push_constants_size, vk::Device device);
		void Execute(uint32_t compute_queue_index, size_t total_compute_count, const std::vector<vk::Buffer> &buffers, void *push_constants = nullptr);

	private:
		const std::string ShaderFileName;
		size_t BufferCount;
		size_t PushConstantsSize;
		vk::DescriptorSetLayout DescriptorSetLayout = nullptr;
		vk::Device Device = nullptr;
		vk::PhysicalDevice PhysicalDevice = nullptr;
		vk::ShaderModule ShaderModule = nullptr;
		std::vector<vk::DescriptorSet> DescriptorSets;
		vk::PipelineLayout PipelineLayout = nullptr;
		vk::Pipeline Pipeline = nullptr;

		vk::ShaderModule CreateShaderModule();
		vk::DescriptorSetLayout DescribeShader();
		vk::Result CreatePipeline(size_t push_constants_size);

		std::vector<vk::DescriptorSet> AllocateDescriptorSets();
		void UpdateDescriptorSets(const std::vector<vk::DescriptorSet> &descriptor_set, const std::vector<vk::Buffer> &buffers);
		
		GPUComputeShader(const GPUComputeShader &other) = delete;
		GPUComputeShader(const GPUComputeShader &&other) = delete;
	};
}
