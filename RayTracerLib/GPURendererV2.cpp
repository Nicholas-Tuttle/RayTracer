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

static const vk::DeviceSize RayBufferSize = 1024 * 1024;

GPURendererV2::GPURendererV2(const Camera &camera, unsigned int samples) : camera(camera), samples(samples)
{
	ElapsedTimer performance_timer;

	std::cout << "Perf counter created" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << std::endl;

	if (!VKUtils::VerifyInstanceLayers(VKUtils::DebugInstanceLayers) ||
		!VKUtils::VerifyInstanceExtensions(VKUtils::DebugInstanceExtensions))
	{
		throw std::exception("Failed to verify instance layers or extensions");
	}

	instance = VKUtils::CreateHeadlessVulkanInstance(VKUtils::DebugInstanceLayers, VKUtils::DebugInstanceExtensions,
		(void *)&VKUtils::VulkanDebugPrintfInstanceItem);

	debugMessenger = VKUtils::CreateDebugMessenger(instance, VKUtils::DefaultVulkanDebugCallback);

	std::vector<vk::PhysicalDevice> devices;
	VKUtils::EnumerateDevices(instance, devices);
	if (devices.size() == 0)
	{
		throw std::exception("Failed to enumerate devices");
	}

	PhysicalDevice = devices[0];

	if (!VKUtils::VerifyDeviceExtensions(PhysicalDevice, VKUtils::DebugDeviceExtensions))
	{
		throw std::exception("Failed to verify device layers or extensions");
	}

	if (vk::Result::eSuccess != VKUtils::GetBestComputeQueue(PhysicalDevice, ComputeQueueIndex))
	{
		throw std::exception("Failed to get best compute queue");
	}

	device = VKUtils::CreateDevice(PhysicalDevice, VKUtils::DebugDeviceExtensions, ComputeQueueIndex);

	BufferData.resize((size_t)GPUBufferBindings::GPUBufferBindingCount);

	size_t total_rays = camera.Resolution().X * camera.Resolution().Y * samples;
	size_t ray_buffer_count = total_rays / RayBufferSize + ((total_rays % RayBufferSize) > 0 ? 1 : 0);
	std::cout << "Total ray buffers needed for " << total_rays << " rays with buffer of size " << RayBufferSize << ": " << ray_buffer_count << std::endl;

	BufferData[(int)GPUBufferBindings::ray_buffer].buffer_size = sizeof(GPURay) * RayBufferSize;
	BufferData[(int)GPUBufferBindings::ray_buffer].usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::ray_buffer].descriptor_type = vk::DescriptorType::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::ray_buffer].data_pointers = &gpu_ray_buffers;

	BufferData[(int)GPUBufferBindings::ray_buffer].resize(ray_buffer_count);

	BufferData[(int)GPUBufferBindings::intersection_buffer].buffer_size = sizeof(GPUIntersection) * RayBufferSize;
	BufferData[(int)GPUBufferBindings::intersection_buffer].usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::intersection_buffer].descriptor_type = vk::DescriptorType::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::intersection_buffer].data_pointers = &gpu_intersection_buffers;

	BufferData[(int)GPUBufferBindings::intersection_buffer].resize(ray_buffer_count);

	if (vk::Result::eSuccess != CreateAndMapMemories(ComputeQueueIndex))
	{
		throw std::exception("Failed to create and map memories for compute shader");
	}

	for (const auto buffer : gpu_ray_buffers)
	{
		if (nullptr == buffer)
		{
			throw std::exception("Ray initialization buffer data for GPU compute failed to allocate");
		}
	}

	for (const auto buffer : gpu_intersection_buffers)
	{
		if (nullptr == buffer)
		{
			throw std::exception("Ray intersection buffer data for GPU compute failed to allocate");
		}
	}

	std::cout << "GPU Renderer initialization took" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << std::endl;
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
		for (size_t i = 0; i < buffer_data.data_pointers->size(); i++)
		{
			if (nullptr == ((*buffer_data.data_pointers)[i] = CreateAndMapMemory(queueFamilyIndex, buffer_data.buffer_size, buffer_data.usage_flag_bits,
				buffer_data.buffers[i], buffer_data.device_memories[i])))
			{
				result = vk::Result::eErrorMemoryMapFailed;
			}
		}
	}

	return result;
}

void GPURendererV2::Render(const IScene &scene, std::shared_ptr<IImage> &out_image)
{
	ElapsedTimer performance_timer;

	std::cout << "[RENDER STARTED]" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << std::endl;

	GPURayInitializer ray_initializer(device);
	
	// Ray initialization
	for (size_t i = 0; i < BufferData[(int)GPUBufferBindings::ray_buffer].buffers.size(); i++)
	{
		ray_initializer.Execute(ComputeQueueIndex, camera.Position(), camera.ForwardVector(),
			camera.RightVector(), camera.UpVector(), camera.FocalLengthMM(), camera.SensorWidthMM(),
			camera.Resolution().X, camera.Resolution().Y, samples, 0, RayBufferSize * i, 
			BufferData[(int)GPUBufferBindings::ray_buffer].buffers[i]);

#ifdef _DEBUG
		std::cout << "---- initialized ray values for ray initialization buffer " << i << " ----" << std::endl;
		for (size_t j = 0, max = 0; j < RayBufferSize && max < 100; j++, max++)
		{
			std::cout << std::fixed << std::setprecision(9) << j << ":\t" << ((GPURay *)gpu_ray_buffers[i])[j].direction[0] << "\t\t"
				<< ((GPURay *)gpu_ray_buffers[i])[j].direction[1] << "\t\t" << ((GPURay *)gpu_ray_buffers[i])[j].direction[2] << std::endl;
		}
		std::cout << "---- ----" << std::endl;
#endif
	}

	// Ray Intersection
	GPURayIntersector ray_intersector(device);
	for (size_t i = 0; i < BufferData[(int)GPUBufferBindings::ray_buffer].buffers.size(); i++)
	{
		ray_intersector.Execute(ComputeQueueIndex, RayBufferSize, 
			BufferData[(int)GPUBufferBindings::ray_buffer].buffers[i],
			BufferData[(int)GPUBufferBindings::intersection_buffer].buffers[i]);

#ifdef _DEBUG
		std::cout << "---- Calculated intersections for ray buffer " << i << " ----" << std::endl;
		for (size_t j = 0, max = 0; j < RayBufferSize && max < 100; j++, max++)
		{
			std::cout << std::fixed << std::setprecision(9) << j << ":\tMaterial ID: " 
				<< ((GPUIntersection *)gpu_intersection_buffers[i])[j].material_id << std::endl;
		}
		std::cout << "---- ----" << std::endl;
#endif
	}

	// Material Calculation will be here
	GPUWorldMaterial world_material(device);
	for (size_t i = 0; i < gpu_ray_buffers.size(); i++)
	{
		world_material.Execute(ComputeQueueIndex, RayBufferSize, 
			BufferData[(int)GPUBufferBindings::intersection_buffer].buffers[i],
			BufferData[(int)GPUBufferBindings::ray_buffer].buffers[i]);
#ifdef _DEBUG
		std::cout << "---- Calculated world material for ray buffer " << i << " ----" << std::endl;
		std::cout << "---- ----" << std::endl;
#endif
	}

	// Sample accumulation or re-queueing
	Image *output_image = new Image(camera.Resolution());
	for (size_t i = 0; i < gpu_ray_buffers.size() && i < gpu_intersection_buffers.size(); i++)
	{
		for (size_t j = 0; j < RayBufferSize; j++)
		{
			GPURay &ray = ((GPURay *)(gpu_ray_buffers[i]))[j];
			GPUIntersection &intersection = ((GPUIntersection *)(gpu_intersection_buffers[i]))[j];
			float *new_ray_direction = ray.direction;
			if (0 == new_ray_direction[0] && 0 == new_ray_direction[1] && 0 == new_ray_direction[2])
			{
				// Send to sample accumulator
				float *intersection_color = intersection.ray_color;
				output_image->SetPixelColor(intersection.pixelXIndex, intersection.pixelYIndex,
					Color(intersection_color[0], intersection_color[1], intersection_color[2], 1.0f));
			}
			else
			{
				// Put it back on the queue
				std::cout << "Requeueing ray should not happen yet" << std::endl;
				exit(1);
			}
		}
	}

	out_image = std::shared_ptr<IImage>(output_image);

	std::cout << "[RENDER TIME]" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << std::endl;
}

GPURendererV2::~GPURendererV2()
{
	VKUtils::DestroyDebugMessenger(instance, debugMessenger);
}

