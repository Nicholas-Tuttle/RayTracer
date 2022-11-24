#include "GPURendererV2.h"
#include <vulkan/vulkan.hpp>
#include <iostream>
#include "ElapsedTimer.h"
#include "VulkanUtils.h"
#include "GPURayInitializer.h"
#include "GPURayIntersector.h"
#include "GPUWorldMaterial.h"
#include "GPUDiffuseMaterial.h"
#include "GPUSampleAccumulator.h"
#include "GPUStructs.h"

using RayTracer::GPURendererV2;
using RayTracer::Camera;
using RayTracer::IScene;
using RayTracer::IImage;
using RayTracer::ElapsedTimer;

using RayTracer::GPURayInitializer;
using RayTracer::GPURayIntersector;
using RayTracer::GPURay;

using RayTracer::IGPUMaterial;
using RayTracer::GPUWorldMaterial;
using RayTracer::GPUDiffuseMaterial;

using RayTracer::GPUSampleAccumulator;

GPURendererV2::GPURendererV2(const Camera &camera) : camera(camera)
{
	ElapsedTimer performance_timer;

	std::cout << "Perf counter created" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << std::endl;

	if (!VKUtils::VerifyInstanceLayers(VKUtils::DebugInstanceLayers) ||
		!VKUtils::VerifyInstanceExtensions(VKUtils::DebugInstanceExtensions))
	{
		return;
	}

	instance = VKUtils::CreateHeadlessVulkanInstance(VKUtils::DebugInstanceLayers, VKUtils::DebugInstanceExtensions,
		(void *)&VKUtils::VulkanDebugPrintfInstanceItem);

	debugMessenger = VKUtils::CreateDebugMessenger(instance, VKUtils::DefaultVulkanDebugCallback);

	std::vector<vk::PhysicalDevice> devices;
	VKUtils::EnumerateDevices(instance, devices);
	if (devices.size() == 0)
	{
		return;
	}

	PhysicalDevice = devices[0];

	if (vk::Result::eSuccess != VKUtils::GetBestComputeQueue(PhysicalDevice, ComputeQueueIndex))
	{
		return;
	}

	device = VKUtils::CreateDevice(PhysicalDevice, ComputeQueueIndex);

	BufferData.resize((size_t)GPUBufferBindings::GPUBufferBindingCount);

	BufferData[(int)GPUBufferBindings::ray_buffer].buffer_size = sizeof(GPURay) * camera.Resolution().X * camera.Resolution().Y;
	BufferData[(int)GPUBufferBindings::ray_buffer].usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::ray_buffer].descriptor_type = vk::DescriptorType::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::ray_buffer].data_pointer = (void **)&gpu_ray_buffer;

	BufferData[(int)GPUBufferBindings::intersection_buffer].buffer_size = sizeof(GPUIntersection) * camera.Resolution().X * camera.Resolution().Y;
	BufferData[(int)GPUBufferBindings::intersection_buffer].usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::intersection_buffer].descriptor_type = vk::DescriptorType::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::intersection_buffer].data_pointer = (void **)&gpu_intersection_buffer;

	if (vk::Result::eSuccess != CreateAndMapMemories(ComputeQueueIndex))
	{
		throw std::exception("Failed to create and map memories for compute shader");
	}

	if (nullptr == gpu_ray_buffer || nullptr == gpu_intersection_buffer)
	{
		throw std::exception("Buffer data for GPU compute failed to allocate");
	}

	std::cout << "GPU Renderer initialization took" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << std::endl;
}

GPURendererV2::GPURendererV2(const Camera &camera, HINSTANCE hInstance) : GPURendererV2(camera)
{

}

void *GPURendererV2::CreateAndMapMemory(uint32_t queueFamilyIndex, const vk::DeviceSize memorySize, const vk::BufferUsageFlags usage_flags,
	vk::Buffer &vk_buffer, vk::DeviceMemory &device_memory)
{
	if (memorySize == 0)
	{
		return nullptr;
	}

	vk::PhysicalDeviceMemoryProperties properties = {};
	PhysicalDevice.getMemoryProperties(&properties);

	// set memoryTypeIndex to an invalid entry in the properties.memoryTypes array
	uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;

	for (uint32_t k = 0; k < properties.memoryTypeCount; k++)
	{
		
		if ((vk::MemoryPropertyFlagBits::eHostVisible & properties.memoryTypes[k].propertyFlags) &&
			(vk::MemoryPropertyFlagBits::eHostCoherent & properties.memoryTypes[k].propertyFlags) &&
			(vk::MemoryPropertyFlagBits::eHostCached & properties.memoryTypes[k].propertyFlags) &&
			(memorySize < properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size))
		{
			memoryTypeIndex = k;
			break;
		}
	}

	if (memoryTypeIndex == VK_MAX_MEMORY_TYPES)
	{
		return nullptr;
	}

	vk::MemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.allocationSize = memorySize;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	device_memory = device.allocateMemory(memoryAllocateInfo);

	vk::BufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.size = memorySize;
	bufferCreateInfo.usage = usage_flags;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	bufferCreateInfo.queueFamilyIndexCount = 1;
	bufferCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;

	vk_buffer = device.createBuffer(bufferCreateInfo);
	device.bindBufferMemory(vk_buffer, device_memory, 0);
	
	return device.mapMemory(device_memory, 0, memorySize);
}

vk::Result GPURendererV2::CreateAndMapMemories(uint32_t queueFamilyIndex)
{
	vk::Result result = vk::Result::eSuccess;

	for (auto &buffer_data : BufferData)
	{
		if (nullptr == (*buffer_data.data_pointer = CreateAndMapMemory(queueFamilyIndex, buffer_data.buffer_size, buffer_data.usage_flag_bits,
			buffer_data.buffer, buffer_data.device_memory)))
		{
			result = vk::Result::eErrorMemoryMapFailed;
		}
	}

	return result;
}

void GPURendererV2::Render(unsigned int samples, const IScene &scene, std::shared_ptr<IImage> &out_image)
{
	ElapsedTimer performance_timer;

	std::cout << "[RENDER STARTED]" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << std::endl;

	GPURayInitializer ray_initializer(device, camera, camera.Resolution().X, camera.Resolution().Y, 
		gpu_ray_buffer, BufferData[(int)GPUBufferBindings::ray_buffer].buffer);
	ray_initializer.Execute(ComputeQueueIndex);

	for (size_t i = 0; i < camera.Resolution().X * camera.Resolution().Y; i++)
	{
		std::cout << gpu_ray_buffer[i].direction[0] << " "
			<< gpu_ray_buffer[i].direction[1] << " "
			<< gpu_ray_buffer[i].direction[2] << " "
			<< gpu_ray_buffer[i].direction[3] << std::endl;
	}

	GPURayIntersector ray_intersector(device, camera.Resolution().X * camera.Resolution().Y, gpu_ray_buffer, gpu_intersection_buffer, 
		BufferData[(int)GPUBufferBindings::ray_buffer].buffer, BufferData[(int)GPUBufferBindings::intersection_buffer].buffer);
	ray_intersector.Execute(ComputeQueueIndex);

	std::cout << "[RENDER TIME]" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << std::endl;
}

GPURendererV2::~GPURendererV2()
{
	VKUtils::DestroyDebugMessenger(instance, debugMessenger);
}

