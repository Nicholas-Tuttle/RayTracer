#include "GPUSampleAccumulator.h"
#include "VulkanUtils.h"

using RayTracer::GPUSampleAccumulator;
using RayTracer::VKUtils;

const static size_t shader_local_size_x = 1024;

GPUSampleAccumulator::GPUSampleAccumulator(vk::Device device)
	: SampleAccumulatorPushConstants(), GPUComputeShader("GPUSampleAccumulator.comp.spv", device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << std::endl;
#endif

	DescriptorSetLayout = DescribeShader();
	if (DescriptorSetLayout == static_cast<vk::DescriptorSetLayout>(nullptr))
	{
		throw std::exception("Failed to describe shader");
	}

	if (vk::Result::eSuccess != CreatePipeline())
	{
		throw std::exception("Failed to create pipeline layout");
	}

	DescriptorSets = AllocateDescriptorSets();
}

vk::DescriptorSetLayout GPUSampleAccumulator::DescribeShader()
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
	descriptorSetLayoutCreateInfo.bindingCount = ARRAYSIZE(descriptorSetLayoutBindings);
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;

	return Device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
}

vk::Result GPUSampleAccumulator::CreatePipeline()
{
	vk::PushConstantRange push_constants;
	push_constants.offset = 0;
	push_constants.size = sizeof(SampleAccumulatorPushConstants);
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

std::vector<vk::DescriptorSet> GPUSampleAccumulator::AllocateDescriptorSets()
{
	vk::DescriptorPoolSize descriptorPoolSizes[2] = {};

	descriptorPoolSizes[0].type = vk::DescriptorType::eStorageBuffer;
	descriptorPoolSizes[0].descriptorCount = 1;

	descriptorPoolSizes[1].type = vk::DescriptorType::eStorageBuffer;
	descriptorPoolSizes[1].descriptorCount = 1;

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = ARRAYSIZE(descriptorPoolSizes);
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

void GPUSampleAccumulator::UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_sample_buffer)
{
	vk::DescriptorBufferInfo writeDescriptorSetBufferInfo[2] = {};
	vk::WriteDescriptorSet writeDescriptorSet[2] = {};

	writeDescriptorSetBufferInfo[0].buffer = input_gpu_intersection_buffer;
	writeDescriptorSetBufferInfo[0].offset = 0;
	writeDescriptorSetBufferInfo[0].range = VK_WHOLE_SIZE;

	writeDescriptorSet[0].dstSet = descriptorSet[0];
	writeDescriptorSet[0].dstBinding = 0;
	writeDescriptorSet[0].descriptorCount = 1;
	writeDescriptorSet[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	writeDescriptorSet[0].pBufferInfo = &writeDescriptorSetBufferInfo[0];

	writeDescriptorSetBufferInfo[1].buffer = output_gpu_sample_buffer;
	writeDescriptorSetBufferInfo[1].offset = 0;
	writeDescriptorSetBufferInfo[1].range = VK_WHOLE_SIZE;

	writeDescriptorSet[1].dstSet = descriptorSet[0];
	writeDescriptorSet[1].dstBinding = 1;
	writeDescriptorSet[1].descriptorCount = 1;
	writeDescriptorSet[1].descriptorType = vk::DescriptorType::eStorageBuffer;
	writeDescriptorSet[1].pBufferInfo = &writeDescriptorSetBufferInfo[1];

	Device.updateDescriptorSets(ARRAYSIZE(writeDescriptorSet), writeDescriptorSet, 0, nullptr);
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

	UpdateDescriptorSets(DescriptorSets, input_gpu_intersection_buffer, output_gpu_sample_buffer);

	vk::CommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.queueFamilyIndex = compute_queue_index;

	vk::CommandPool commandPool = Device.createCommandPool(commandPoolCreateInfo);;

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocateInfo.commandBufferCount = 1;

	std::vector<vk::CommandBuffer> commandBuffers = {};
	commandBuffers = Device.allocateCommandBuffers(commandBufferAllocateInfo);

	vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	uint32_t group_count_x = (uint32_t)std::ceil((float)incoming_ray_count / (float)shader_local_size_x);
	commandBuffers[0].begin(commandBufferBeginInfo);
	commandBuffers[0].bindPipeline(vk::PipelineBindPoint::eCompute, Pipeline);
	commandBuffers[0].bindDescriptorSets(vk::PipelineBindPoint::eCompute, PipelineLayout, 0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 0, 0);
	commandBuffers[0].pushConstants(PipelineLayout, vk::ShaderStageFlagBits::eCompute, 0,
		sizeof(SampleAccumulatorPushConstants), (void *)&SampleAccumulatorPushConstants);
	commandBuffers[0].dispatch(group_count_x, 1, 1);
	commandBuffers[0].end();

	vk::Queue queue = Device.getQueue(compute_queue_index, 0);

	vk::SubmitInfo submitInfo = {};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = commandBuffers.data();

	vk::Result queue_submit_result = queue.submit(1, &submitInfo, VK_NULL_HANDLE);
	if (vk::Result::eSuccess == queue_submit_result)
	{
		queue.waitIdle();
	}

	Device.destroyCommandPool(commandPool);
}

