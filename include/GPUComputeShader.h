#pragma once

#include <vulkan/vulkan.hpp>

namespace RayTracer
{
	class GPUComputeShader
	{
	protected:
		GPUComputeShader(const std::string &shader_file_name, vk::Device device);

		vk::Device Device = nullptr;
		vk::PhysicalDevice PhysicalDevice = nullptr;
		vk::ShaderModule ShaderModule = nullptr;
		vk::DescriptorSetLayout DescriptorSetLayout = nullptr;
		std::vector<vk::DescriptorSet> DescriptorSets;
		vk::PipelineLayout PipelineLayout = nullptr;
		vk::Pipeline Pipeline = nullptr;

	private:
		const std::string ShaderFileName;
		vk::ShaderModule CreateShaderModule();
		GPUComputeShader(const GPUComputeShader &other) = delete;
		GPUComputeShader(const GPUComputeShader &&other) = delete;
	};
}
