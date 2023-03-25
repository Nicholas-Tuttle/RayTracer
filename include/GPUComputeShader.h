#pragma once

#include <vulkan/vulkan.hpp>

#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPUComputeShader
	{
	public:
		static void WriteCalculationCompleteEvent(vk::Device device, vk::CommandBuffer &buffer);
	protected:
		GPUComputeShader(const std::string &shader_file_name, uint32_t compute_queue_index, size_t buffer_count, size_t push_constants_size, vk::Device device, const std::vector<vk::Buffer> &buffers, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
		~GPUComputeShader();
		void WriteCommandBuffer(vk::CommandBuffer &command_buffer, size_t total_compute_count, void *push_constants = nullptr);
	private:
		const std::string ShaderFileName;
		size_t BufferCount;
		size_t PushConstantsSize;
		uint32_t ComputeQueueIndex;
		vk::Device Device = nullptr;
		vk::DescriptorSetLayout DescriptorSetLayout = nullptr;
		vk::PhysicalDevice PhysicalDevice = nullptr;
		vk::ShaderModule ShaderModule = nullptr;
		vk::DescriptorPool DescriptorPool = nullptr;
		std::vector<vk::DescriptorSet> DescriptorSets;
		vk::PipelineLayout PipelineLayout = nullptr;
		vk::Pipeline Pipeline = nullptr;

		vk::ShaderModule CreateShaderModule();
		vk::DescriptorSetLayout DescribeShader();
		vk::Result CreatePipeline(size_t push_constants_size);
		std::vector<vk::DescriptorSet> AllocateDescriptorSets();
		vk::CommandPool CreateCommandPool();
		std::vector<vk::CommandBuffer> CreateCommandBuffers();
		void UpdateDescriptorSets(const std::vector<vk::DescriptorSet> &descriptor_set, const std::vector<vk::Buffer> &buffers);
		
		GPUComputeShader(const GPUComputeShader &other) = delete;
		GPUComputeShader(const GPUComputeShader &&other) = delete;

		const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
	};
}
