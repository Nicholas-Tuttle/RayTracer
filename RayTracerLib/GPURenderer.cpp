#include "GPURenderer.h"
#include <vulkan/vulkan.hpp>
#include <iostream>
#include "ElapsedTimer.h"
#include "VulkanUtils.h"
#include "GPURayInitializer.h"
#include "GPURayIntersector.h"
#include "GPUMaterialCalculator.h"
#include "GPUSampleAccumulator.h"
#include "GPUStructs.h"

#include "DiffuseBSDF.h"

using RayTracer::GPURenderer;
using RayTracer::Camera;
using RayTracer::IScene;
using RayTracer::IImage;
using RayTracer::ElapsedTimer;

using RayTracer::GPURayInitializer;
using RayTracer::GPURayIntersector;
using RayTracer::GPUMaterialCalculator;
using RayTracer::GPUSampleAccumulator;

using RayTracer::GPURay;
using RayTracer::GPUSphere;
using RayTracer::GPUDiffuseMaterialParameters;

using RayTracer::DiffuseBSDF;

GPURenderer::GPURenderer(const Camera &camera, unsigned int samples, const IScene &scene) 
	: camera(camera), samples(samples), scene(scene)
{
	ElapsedTimer performance_timer;

	std::cout << "Perf counter created" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << "\n";

	RayBufferSize = camera.Resolution().X * camera.Resolution().Y;

	if (!VKUtils::VerifyInstanceLayers(VKUtils::DebugInstanceLayers) ||
		!VKUtils::VerifyInstanceExtensions(VKUtils::DebugInstanceExtensions))
	{
		throw std::exception("Failed to verify instance layers or extensions");
	}

	instance = VKUtils::CreateHeadlessVulkanInstance(VKUtils::DebugInstanceLayers, VKUtils::DebugInstanceExtensions,
		(void *)&VKUtils::VulkanDebugPrintfInstanceItem);

#if defined _DEBUG
	debugMessenger = VKUtils::CreateDebugMessenger(instance, VKUtils::DefaultVulkanDebugCallback);
#endif

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

	BufferData[(int)GPUBufferBindings::ray_buffer].buffer_size = sizeof(GPURay) * RayBufferSize;
	BufferData[(int)GPUBufferBindings::ray_buffer].usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::ray_buffer].descriptor_type = vk::DescriptorType::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::ray_buffer].data_pointer = &gpu_ray_buffer;

	BufferData[(int)GPUBufferBindings::intersection_buffer].buffer_size = sizeof(GPUIntersection) * RayBufferSize;
	BufferData[(int)GPUBufferBindings::intersection_buffer].usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::intersection_buffer].descriptor_type = vk::DescriptorType::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::intersection_buffer].data_pointer = &gpu_intersection_buffer;

	std::vector<GPUSphere> spheres;
	for (const auto &object : scene.Objects())
	{
		const Sphere *sphere = dynamic_cast<const Sphere *>(object);
		if (nullptr != sphere)
		{
			spheres.emplace_back(sphere);
		}
	}

	BufferData[(int)GPUBufferBindings::sphere_buffer].buffer_size = sizeof(GPUSphere) * spheres.size();
	BufferData[(int)GPUBufferBindings::sphere_buffer].usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::sphere_buffer].descriptor_type = vk::DescriptorType::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::sphere_buffer].data_pointer = &gpu_sphere_buffer;

	BufferData[(int)GPUBufferBindings::sample_buffer].buffer_size = sizeof(GPUSample) * RayBufferSize;
	BufferData[(int)GPUBufferBindings::sample_buffer].usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::sample_buffer].descriptor_type = vk::DescriptorType::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::sample_buffer].data_pointer = &gpu_sample_buffer;

	std::vector<GPUDiffuseMaterialParameters> diffuse_material_parameters;
	for (const auto &material : scene.Materials())
	{
		const DiffuseBSDF *diffuse_material = dynamic_cast<const DiffuseBSDF *>(material);
		if (nullptr != diffuse_material)
		{
			diffuse_material_parameters.emplace_back(diffuse_material);
		}
	}

	BufferData[(int)GPUBufferBindings::diffuse_material_parameters].buffer_size = sizeof(GPUDiffuseMaterialParameters) * diffuse_material_parameters.size();
	BufferData[(int)GPUBufferBindings::diffuse_material_parameters].usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::diffuse_material_parameters].descriptor_type = vk::DescriptorType::eStorageBuffer;
	BufferData[(int)GPUBufferBindings::diffuse_material_parameters].data_pointer = &gpu_diffuse_material_parameters_buffer;

	if (vk::Result::eSuccess != CreateAndMapMemories(ComputeQueueIndex))
	{
		throw std::exception("Failed to create and map memories for compute shader");
	}

	if (nullptr == gpu_ray_buffer)
	{
		throw std::exception("Ray initialization buffer data for GPU compute failed to allocate");
	}

	if (nullptr == gpu_intersection_buffer)
	{
		throw std::exception("Ray intersection buffer data for GPU compute failed to allocate");
	}

	if (nullptr == gpu_sphere_buffer)
	{
		throw std::exception("Sphere buffer data for GPU compute failed to allocate");
	}

	if (nullptr == gpu_sample_buffer)
	{
		throw std::exception("Sample buffer data for GPU compute failed to allocate");
	}

	if (nullptr == gpu_diffuse_material_parameters_buffer)
	{
		throw std::exception("Diffuse material parameters for GPU compute failed to allocate");
	}

	memcpy(gpu_sphere_buffer, spheres.data(), spheres.size() * sizeof(GPUSphere));
	memcpy(gpu_diffuse_material_parameters_buffer, diffuse_material_parameters.data(), diffuse_material_parameters.size() * sizeof(GPUDiffuseMaterialParameters));

	std::cout << "GPU Renderer initialization took" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << "\n";
}

void *GPURenderer::CreateAndMapMemory(uint32_t queueFamilyIndex, const vk::DeviceSize memorySize, const vk::BufferUsageFlags usage_flags,
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

vk::Result GPURenderer::CreateAndMapMemories(uint32_t queueFamilyIndex)
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

static bool RayCalculationNeeded(const GPURay * const rays, size_t rays_length, size_t current_bounce, size_t max_bounce)
{
	if (current_bounce >= max_bounce)
	{
		return false;
	}

	for (size_t i = 0; i < rays_length; i++)
	{
		if (rays[i].direction[0] != 0 ||
			rays[i].direction[1] != 0 ||
			rays[i].direction[2] != 0)
		{
			return true;
		}
	}

	return false;
}

void GPURenderer::Render(std::shared_ptr<IImage> &out_image)
{
	ElapsedTimer performance_timer;

	std::cout << "[RENDER STARTED]" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << "\n";

	GPURayInitializer ray_initializer(device);
	GPURayIntersector ray_intersector(device, scene);
	GPUMaterialCalculator material_calculator(device);
	GPUSampleAccumulator sample_accumulator(device);

	const size_t max_bounces = 12;

	for (size_t i = 0; i < samples; i++)
	{
		std::cout << "############## CALCULATING SAMPLE " << i + 1 << " OF " << samples << " ##############\n";

		ray_initializer.Execute(ComputeQueueIndex, camera, i, BufferData[(int)GPUBufferBindings::ray_buffer].buffer, BufferData[(int)GPUBufferBindings::intersection_buffer].buffer);

#if defined _DEBUG && false
		std::cout << "---- initialized ray values for ray initialization buffer " << i << " ----" << "\n";
		for (size_t j = 0, max = 0; j < RayBufferSize && max < 100; j++, max++)
		{
			std::cout << std::fixed << std::setprecision(9) << j << ":\t" << ((GPURay *)gpu_ray_buffer)[j].direction[0] << "\t\t"
				<< ((GPURay *)gpu_ray_buffer)[j].direction[1] << "\t\t" << ((GPURay *)gpu_ray_buffer)[j].direction[2] << "\n";
		}
		std::cout << "---- ----" << "\n";
#endif

		bool ray_calculation_needed = true;
		size_t current_bounce = 1;

		while (ray_calculation_needed)
		{
			ray_intersector.Execute(ComputeQueueIndex, RayBufferSize,
				BufferData[(int)GPUBufferBindings::ray_buffer].buffer,
				BufferData[(int)GPUBufferBindings::intersection_buffer].buffer,
				BufferData[(int)GPUBufferBindings::sphere_buffer].buffer);

	#if defined _DEBUG && false
			std::cout << "---- Calculated intersections for ray buffer " << i << " ----" << "\n";
			for (size_t j = 0, max = 0; j < RayBufferSize && max < 100; j++, max++)
			{
				std::cout << std::fixed << std::setprecision(9) << j << ":\tMaterial ID: "
					<< ((GPUIntersection *)gpu_intersection_buffer)[j].material_id << "\n";
			}
			std::cout << "---- ----" << "\n";
	#endif

			material_calculator.Execute(ComputeQueueIndex, RayBufferSize,
				BufferData[(int)GPUBufferBindings::intersection_buffer].buffer,
				BufferData[(int)GPUBufferBindings::ray_buffer].buffer,
				BufferData[(int)GPUBufferBindings::diffuse_material_parameters].buffer);

	#if defined _DEBUG && false
			std::cout << "---- Calculated materials for ray buffer " << i << " ----" << "\n";
	#endif

			ray_calculation_needed = RayCalculationNeeded((const GPURay * const)gpu_ray_buffer, RayBufferSize, current_bounce, max_bounces);

			current_bounce++;
		}

		sample_accumulator.Execute(ComputeQueueIndex, RayBufferSize,
			BufferData[(int)GPUBufferBindings::intersection_buffer].buffer,
			BufferData[(int)GPUBufferBindings::sample_buffer].buffer);

#if defined _DEBUG && false
		std::cout << "---- Accumulated samples for ray buffer " << i << " ----" << "\n";
#endif
	}

#if defined _DEBUG && true
	std::cout << "---- Finalizing Samples ----" << "\n";
#endif
	// Finalize the samples
	sample_accumulator.Execute(ComputeQueueIndex, RayBufferSize,
		BufferData[(int)GPUBufferBindings::intersection_buffer].buffer,
		BufferData[(int)GPUBufferBindings::sample_buffer].buffer,
		true, samples);

	Image *output_image = new Image(camera.Resolution());

	for (size_t y = 0; y < camera.Resolution().Y; y++)
	{
		size_t offset = y * camera.Resolution().X;
		for (size_t x = 0; x < camera.Resolution().X; x++)
		{
			const GPUSample &sample = ((GPUSample *)gpu_sample_buffer)[offset + x];
			output_image->SetPixelColor(x, y, Color(sample.color[0],
				sample.color[1], sample.color[2], sample.color[3]));
		}
	}

	out_image = std::shared_ptr<IImage>(output_image);

	std::cout << "[RENDER TIME]" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << "\n";
}

GPURenderer::~GPURenderer()
{
#if defined _DEBUG
	VKUtils::DestroyDebugMessenger(instance, debugMessenger);
#endif
}

