#include "GPUComputeShader.h"
#include "VulkanUtils.h"
#include <iostream>

using RayTracer::GPUComputeShader;

GPUComputeShader::GPUComputeShader(const std::string &shader_file_name, vk::Device device)
	: ShaderFileName(shader_file_name), Device(device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << std::endl;
#endif

	ShaderModule = CreateShaderModule();
	if (ShaderModule == static_cast<vk::ShaderModule>(nullptr))
	{
		throw std::exception("Failed to create shader module");
	}
}

vk::ShaderModule GPUComputeShader::CreateShaderModule()
{
	std::vector<uint8_t> shaderCode = VKUtils::ReadShaderFileToBytes(ShaderFileName);
	if (0 == shaderCode.size())
	{
		return nullptr;
	}

	vk::ShaderModuleCreateInfo shaderModuleCreateInfo;
	shaderModuleCreateInfo.codeSize = shaderCode.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t *>(shaderCode.data());

	return Device.createShaderModule(shaderModuleCreateInfo);
}

