#include <GPURayInitializer.h>
#include "VulkanUtils.h"

using RayTracer::GPURayInitializer;
using RayTracer::Camera;
using RayTracer::GPURay;
using RayTracer::VKUtils;

const static size_t shader_local_size_x = 1024;

GPURayInitializer::GPURayInitializer(vk::Device device, const Camera &camera, size_t resolution_x, size_t resolution_y,
	GPURay *gpu_ray_buffer, vk::Buffer output_ray_buffer)
	: ResolutionX(resolution_x), ResolutionY(resolution_y), OutputRayBuffer(output_ray_buffer), CameraDataPushConstants(),
	GPUComputeShader("GPURayInitializer.comp.spv", device)
{
	std::cout << __FUNCTION__ << std::endl;

	CameraDataPushConstants.camera_origin[0] = camera.Position().X;
	CameraDataPushConstants.camera_origin[1] = camera.Position().Y;
	CameraDataPushConstants.camera_origin[2] = camera.Position().Z;

	CameraDataPushConstants.forward_vector[0] = camera.ForwardVector().X;
	CameraDataPushConstants.forward_vector[1] = camera.ForwardVector().Y;
	CameraDataPushConstants.forward_vector[2] = camera.ForwardVector().Z;

	CameraDataPushConstants.right_vector[0] = camera.RightVector().X;
	CameraDataPushConstants.right_vector[1] = camera.RightVector().Y;
	CameraDataPushConstants.right_vector[2] = camera.RightVector().Z;

	CameraDataPushConstants.up_vector[0] = camera.UpVector().X;
	CameraDataPushConstants.up_vector[1] = camera.UpVector().Y;
	CameraDataPushConstants.up_vector[2] = camera.UpVector().Z;

	CameraDataPushConstants.focal_length_mm = camera.FocalLengthMM();
	CameraDataPushConstants.sensor_width_mm = camera.SensorWidthMM();

	CameraDataPushConstants.resolution_x = resolution_x;
	CameraDataPushConstants.resolution_y = resolution_y;

	CameraDataPushConstants.seed = 0;

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

vk::DescriptorSetLayout GPURayInitializer::DescribeShader()
{
	vk::DescriptorSetLayoutBinding descriptorSetLayoutBindings[1] = {};

	descriptorSetLayoutBindings[0].binding = 0;
	descriptorSetLayoutBindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	descriptorSetLayoutBindings[0].descriptorCount = 1;
	descriptorSetLayoutBindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;

	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;

	return Device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
}

vk::Result GPURayInitializer::CreatePipeline()
{
	vk::PushConstantRange push_constants;
	push_constants.offset = 0;
	push_constants.size = sizeof(CameraDataPushConstants);
	push_constants.stageFlags = vk::ShaderStageFlagBits::eCompute;

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &push_constants;

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

std::vector<vk::DescriptorSet> GPURayInitializer::AllocateDescriptorSets()
{
	vk::DescriptorPoolSize descriptorPoolSizes[1] = {};

	descriptorPoolSizes[0].type = vk::DescriptorType::eStorageBuffer;
	descriptorPoolSizes[0].descriptorCount = 1;

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 1;
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

void GPURayInitializer::UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet)
{
	vk::DescriptorBufferInfo writeDescriptorSetBufferInfo[1] = {};
	vk::WriteDescriptorSet writeDescriptorSet[1] = {};

	writeDescriptorSetBufferInfo[0].buffer = OutputRayBuffer;
	writeDescriptorSetBufferInfo[0].offset = 0;
	writeDescriptorSetBufferInfo[0].range = VK_WHOLE_SIZE;

	writeDescriptorSet[0].dstSet = descriptorSet[0];
	writeDescriptorSet[0].dstBinding = 0;
	writeDescriptorSet[0].descriptorCount = 1;
	writeDescriptorSet[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	writeDescriptorSet[0].pBufferInfo = &writeDescriptorSetBufferInfo[0];

	Device.updateDescriptorSets(1, writeDescriptorSet, 0, nullptr);
}

void GPURayInitializer::AllocateAndUpdateDescriptorSets()
{
	DescriptorSets = AllocateDescriptorSets();
	UpdateDescriptorSets(DescriptorSets);
}

void GPURayInitializer::Execute(uint32_t ComputeQueueIndex)
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

	uint32_t group_count_x = (uint32_t)std::ceil((float)(ResolutionX * ResolutionY) / (float)shader_local_size_x);
	commandBuffers[0].begin(commandBufferBeginInfo);
	commandBuffers[0].bindPipeline(vk::PipelineBindPoint::eCompute, Pipeline);
	commandBuffers[0].bindDescriptorSets(vk::PipelineBindPoint::eCompute, PipelineLayout, 0, DescriptorSets.size(), DescriptorSets.data(), 0, 0);
	commandBuffers[0].pushConstants(PipelineLayout, vk::ShaderStageFlagBits::eCompute, 0,
		sizeof(CameraDataPushConstants), (void *)&CameraDataPushConstants);
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
