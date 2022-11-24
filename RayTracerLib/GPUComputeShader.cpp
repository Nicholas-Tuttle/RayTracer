#include "GPUComputeShader.h"
#include "VulkanUtils.h"

using RayTracer::GPUComputeShader;

GPUComputeShader::GPUComputeShader(const std::string &shader_file_name, vk::Device device)
	: ShaderFileName(shader_file_name), Device(device)
{
	// Nothing else to do
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

