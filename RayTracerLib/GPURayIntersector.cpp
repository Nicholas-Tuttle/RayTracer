#include "GPURayIntersector.h"
#include "VulkanUtils.h"

using RayTracer::GPURayIntersector;
using RayTracer::GPURay;
using RayTracer::VKUtils;

const static size_t shader_local_size_x = 1024;

GPURayIntersector::GPURayIntersector(vk::Device device, size_t incoming_ray_count, GPURay *gpu_ray_buffer, 
	GPUIntersection *gpu_intersection_buffer, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_ray_buffer)
	: GPUComputeShader("GPURayIntersector.comp.spv", device), gpu_ray_buffer(gpu_ray_buffer), gpu_intersection_buffer(gpu_intersection_buffer),
	InputRayBuffer(input_gpu_ray_buffer), OutputIntersectionBuffer(output_gpu_ray_buffer), IncomingRayBufferCount(incoming_ray_count)
{
	std::cout << __FUNCTION__ << std::endl;

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

	if (vk::Result::eSuccess != CreatePipeline())
	{
		throw std::exception("Failed to create pipeline layout");
	}

	AllocateAndUpdateDescriptorSets();
}

vk::DescriptorSetLayout GPURayIntersector::DescribeShader()
{
	vk::DescriptorSetLayoutBinding descriptorSetLayoutBindings[2] = {};

	descriptorSetLayoutBindings[0].binding = 0;
	descriptorSetLayoutBindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	descriptorSetLayoutBindings[0].descriptorCount = 1;
	descriptorSetLayoutBindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;

	descriptorSetLayoutBindings[1].binding = 1;
	descriptorSetLayoutBindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
	descriptorSetLayoutBindings[1].descriptorCount = 1;
	descriptorSetLayoutBindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;

	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	descriptorSetLayoutCreateInfo.bindingCount = 2;
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;

	return Device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
}

vk::Result GPURayIntersector::CreatePipeline()
{
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;

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

std::vector<vk::DescriptorSet> GPURayIntersector::AllocateDescriptorSets()
{
	vk::DescriptorPoolSize descriptorPoolSizes[2] = {};

	descriptorPoolSizes[0].type = vk::DescriptorType::eStorageBuffer;
	descriptorPoolSizes[0].descriptorCount = 1;

	descriptorPoolSizes[1].type = vk::DescriptorType::eStorageBuffer;
	descriptorPoolSizes[1].descriptorCount = 1;

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 2;
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;

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

void GPURayIntersector::UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet)
{
	vk::DescriptorBufferInfo writeDescriptorSetBufferInfo[2] = {};
	vk::WriteDescriptorSet writeDescriptorSet[2] = {};

	writeDescriptorSetBufferInfo[0].buffer = InputRayBuffer;
	writeDescriptorSetBufferInfo[0].offset = 0;
	writeDescriptorSetBufferInfo[0].range = VK_WHOLE_SIZE;

	writeDescriptorSet[0].dstSet = descriptorSet[0];
	writeDescriptorSet[0].dstBinding = 0;
	writeDescriptorSet[0].descriptorCount = 1;
	writeDescriptorSet[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	writeDescriptorSet[0].pBufferInfo = &writeDescriptorSetBufferInfo[0];

	writeDescriptorSetBufferInfo[1].buffer = OutputIntersectionBuffer;
	writeDescriptorSetBufferInfo[1].offset = 0;
	writeDescriptorSetBufferInfo[1].range = VK_WHOLE_SIZE;

	writeDescriptorSet[1].dstSet = descriptorSet[0];
	writeDescriptorSet[1].dstBinding = 1;
	writeDescriptorSet[1].descriptorCount = 1;
	writeDescriptorSet[1].descriptorType = vk::DescriptorType::eStorageBuffer;
	writeDescriptorSet[1].pBufferInfo = &writeDescriptorSetBufferInfo[1];

	Device.updateDescriptorSets(2, writeDescriptorSet, 0, nullptr);
}

void GPURayIntersector::AllocateAndUpdateDescriptorSets()
{
	DescriptorSets = AllocateDescriptorSets();
	UpdateDescriptorSets(DescriptorSets);
}

void GPURayIntersector::Execute(uint32_t ComputeQueueIndex)
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.queueFamilyIndex = ComputeQueueIndex;

	vk::CommandPool commandPool = Device.createCommandPool(commandPoolCreateInfo);;

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocateInfo.commandBufferCount = 1;

	std::vector<vk::CommandBuffer> commandBuffers = {};
	commandBuffers = Device.allocateCommandBuffers(commandBufferAllocateInfo);

	vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	uint32_t group_count_x = (uint32_t)std::ceil((float)(IncomingRayBufferCount) / (float)shader_local_size_x);
	commandBuffers[0].begin(commandBufferBeginInfo);
	commandBuffers[0].bindPipeline(vk::PipelineBindPoint::eCompute, Pipeline);
	commandBuffers[0].bindDescriptorSets(vk::PipelineBindPoint::eCompute, PipelineLayout, 0, DescriptorSets.size(), DescriptorSets.data(), 0, 0);
	commandBuffers[0].dispatch(group_count_x, 1, 1);
	commandBuffers[0].end();

	vk::Queue queue = Device.getQueue(ComputeQueueIndex, 0);

	vk::SubmitInfo submitInfo = {};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = commandBuffers.data();

	vk::Result queue_submit_result = queue.submit(1, &submitInfo, VK_NULL_HANDLE);
	if (vk::Result::eSuccess == queue_submit_result)
	{
		queue.waitIdle();
	}
}
