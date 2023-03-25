#include "GPUComputeShader.h"
#include "VulkanUtils.h"
#include <iostream>

using RayTracer::GPUComputeShader;
using RayTracer::PerformanceTracking::PerformanceSession;

const static size_t shader_local_size_x = 1024;

GPUComputeShader::GPUComputeShader(const std::string &shader_file_name, uint32_t compute_queue_index, size_t buffer_count, size_t push_constants_size, vk::Device device, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: ShaderFileName(shader_file_name), ComputeQueueIndex(compute_queue_index), BufferCount(buffer_count), PushConstantsSize(push_constants_size), Device(device), performance_session(session)
{
	TRACE_FUNCTION(performance_session);

	ShaderModule = CreateShaderModule();
	if (ShaderModule == static_cast<vk::ShaderModule>(nullptr))
	{
		throw std::exception("Failed to create shader module");
	}

	DescriptorSetLayout = DescribeShader();
	if (DescriptorSetLayout == static_cast<vk::DescriptorSetLayout>(nullptr))
	{
		throw std::exception("Failed to describe shader");
	}

	if (vk::Result::eSuccess != CreatePipeline(push_constants_size))
	{
		throw std::exception("Failed to create pipeline layout");
	}

	DescriptorSets = AllocateDescriptorSets();

	CommandPool = CreateCommandPool();

	CommandBuffers = CreateCommandBuffers();

	ComputeQueue = Device.getQueue(ComputeQueueIndex, 0);
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

vk::DescriptorSetLayout GPUComputeShader::DescribeShader()
{
	std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
	descriptorSetLayoutBindings.resize(BufferCount, vk::DescriptorSetLayoutBinding());

	for (size_t i = 0; i < descriptorSetLayoutBindings.size(); i++)
	{
		descriptorSetLayoutBindings[i].binding = static_cast<uint32_t>(i);
		descriptorSetLayoutBindings[i].descriptorType = vk::DescriptorType::eStorageBuffer;
		descriptorSetLayoutBindings[i].descriptorCount = 1;
		descriptorSetLayoutBindings[i].stageFlags = vk::ShaderStageFlagBits::eCompute;
	}

	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	return Device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
}

vk::Result GPUComputeShader::CreatePipeline(size_t push_constants_size)
{
	vk::PushConstantRange push_constants;
	push_constants.offset = 0;
	
	if (push_constants_size > 0)
	{
		push_constants.size = static_cast<uint32_t>(push_constants_size);
		push_constants.stageFlags = vk::ShaderStageFlagBits::eCompute;
	}

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;

	if (push_constants_size > 0)
	{
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &push_constants;
	}

	PipelineLayout = Device.createPipelineLayout(pipelineLayoutCreateInfo);
	if (PipelineLayout == static_cast<vk::PipelineLayout>(nullptr))
	{
		return vk::Result::eErrorUnknown;
	}

	vk::ComputePipelineCreateInfo computePipelineCreateInfo;
	computePipelineCreateInfo.stage.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
	computePipelineCreateInfo.stage.stage = vk::ShaderStageFlagBits::eCompute;
	computePipelineCreateInfo.stage.module = ShaderModule;
	computePipelineCreateInfo.stage.pName = "main";
	computePipelineCreateInfo.layout = PipelineLayout;

	return Device.createComputePipelines(VK_NULL_HANDLE, 1, &computePipelineCreateInfo, VK_NULL_HANDLE, &Pipeline);
}

std::vector<vk::DescriptorSet> GPUComputeShader::AllocateDescriptorSets()
{
	std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;
	descriptorPoolSizes.resize(BufferCount, vk::DescriptorPoolSize());

	for (size_t i = 0; i < descriptorPoolSizes.size(); i++)
	{
		descriptorPoolSizes[i].type = vk::DescriptorType::eStorageBuffer;
		descriptorPoolSizes[i].descriptorCount = 1;
	}

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	vk::DescriptorPool descriptorPool = Device.createDescriptorPool(descriptorPoolCreateInfo);
	if (descriptorPool == static_cast<vk::DescriptorPool>(nullptr))
	{
		return std::vector<vk::DescriptorSet>();
	}

	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &DescriptorSetLayout;

	return Device.allocateDescriptorSets(descriptorSetAllocateInfo);
}

vk::CommandPool GPUComputeShader::CreateCommandPool()
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.queueFamilyIndex = ComputeQueueIndex;
	commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	return Device.createCommandPool(commandPoolCreateInfo);
}

std::vector<vk::CommandBuffer> GPUComputeShader::CreateCommandBuffers()
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.commandPool = CommandPool;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocateInfo.commandBufferCount = 1;

	return Device.allocateCommandBuffers(commandBufferAllocateInfo);
}

void GPUComputeShader::UpdateDescriptorSets(const std::vector<vk::DescriptorSet> &descriptor_set, const std::vector<vk::Buffer> &buffers)
{
	TRACE_FUNCTION(performance_session);

	if (buffers.size() != BufferCount)
	{
#ifdef _DEBUG
		std::cout << "Failed to update descriptor sets, incoming buffer count " << buffers.size() << " did not match " << BufferCount << "\n";
#endif
		return;
	}

	std::vector<vk::DescriptorBufferInfo> writeDescriptorSetBufferInfo;
	std::vector<vk::WriteDescriptorSet> writeDescriptorSet;

	writeDescriptorSetBufferInfo.resize(BufferCount, vk::DescriptorBufferInfo());
	writeDescriptorSet.resize(BufferCount, vk::WriteDescriptorSet());

	for (size_t i = 0; i < writeDescriptorSetBufferInfo.size(); i++)
	{
		writeDescriptorSetBufferInfo[i].buffer = buffers[i];
		writeDescriptorSetBufferInfo[i].offset = 0;
		writeDescriptorSetBufferInfo[i].range = VK_WHOLE_SIZE;

		writeDescriptorSet[i].dstSet = descriptor_set[0];
		writeDescriptorSet[i].dstBinding = static_cast<uint32_t>(i);
		writeDescriptorSet[i].descriptorCount = 1;
		writeDescriptorSet[i].descriptorType = vk::DescriptorType::eStorageBuffer;
		writeDescriptorSet[i].pBufferInfo = &writeDescriptorSetBufferInfo[i];
	}

	Device.updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSetBufferInfo.size()), writeDescriptorSet.data(), 0, nullptr);
}

void GPUComputeShader::Execute(size_t total_compute_count, const std::vector<vk::Buffer> &buffers, void *push_constants)
{
	TRACE_FUNCTION(performance_session);

	UpdateDescriptorSets(DescriptorSets, buffers);
	
	vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	uint32_t group_count_x = (uint32_t)std::ceil((float)(total_compute_count) / (float)shader_local_size_x);
	CommandBuffers[0].begin(commandBufferBeginInfo);
	CommandBuffers[0].bindPipeline(vk::PipelineBindPoint::eCompute, Pipeline);
	CommandBuffers[0].bindDescriptorSets(vk::PipelineBindPoint::eCompute, PipelineLayout, 0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 0, 0);
	if (PushConstantsSize > 0)
	{
		CommandBuffers[0].pushConstants(PipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, static_cast<uint32_t>(PushConstantsSize), push_constants);
	}
	CommandBuffers[0].dispatch(group_count_x, 1, 1);
	CommandBuffers[0].end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = CommandBuffers.data();

	{
		// TODO: This takes a long time (relatively) so it should be batched where possible
		TRACE_SCOPE(performance_session, queue_submission);
		vk::Result queue_submit_result = ComputeQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
		if (vk::Result::eSuccess == queue_submit_result)
		{
			ComputeQueue.waitIdle();
		}
	}
}

GPUComputeShader::~GPUComputeShader()
{
	Device.freeCommandBuffers(CommandPool, CommandBuffers);
	Device.destroyCommandPool(CommandPool);
	Device.destroyPipeline(Pipeline);
	Device.destroyDescriptorSetLayout(DescriptorSetLayout);
	Device.destroyShaderModule(ShaderModule);
}
