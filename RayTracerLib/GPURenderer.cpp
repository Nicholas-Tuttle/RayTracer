#include "GPURenderer.h"
#include <vulkan/vulkan.hpp>
#include <iostream>
#include "ElapsedTimer.h"
#include "VulkanUtils.h"
#include "GPUStructs.h"
#include "Mesh.h"

#include "GPURayInitializer.h"
#include "GPURayIntersector.h"
#include "GPUMaterialCalculator.h"
#include "GPUSampleAccumulator.h"
#include "GPUSampleFinalizer.h"

#include "DiffuseBSDF.h"
#include "EmissiveBSDF.h"

#include "PerformanceLogger.h"

using RayTracer::GPURenderer;
using RayTracer::Camera;
using RayTracer::IScene;
using RayTracer::IImage;
using RayTracer::IMaterial;
using RayTracer::ElapsedTimer;
using RayTracer::Sphere;
using RayTracer::Mesh;

using RayTracer::GPURayInitializer;
using RayTracer::GPURayIntersector;
using RayTracer::GPUMaterialCalculator;
using RayTracer::GPUSampleAccumulator;
using RayTracer::GPUSampleFinalizer;

using RayTracer::GPURay;
using RayTracer::GPUSphere;
using RayTracer::GPUSample;
using RayTracer::GPUColor;
using RayTracer::GPUDiffuseMaterialParameters;
using RayTracer::GPUEmissiveMaterialParameters;
using RayTracer::GPUVertex;
using RayTracer::GPUFace;

using RayTracer::DiffuseBSDF;
using RayTracer::EmissiveBSDF;

using RayTracer::PerformanceTracking::PerformanceSession;

GPURenderer::GPURenderer(const GPURendererInitParameters &params)
	: camera(params.camera), samples(params.samples), scene(params.scene), max_bounces(params.max_bounces)
{
	if (params.trace_performance)
	{
		performance_session = std::make_unique<PerformanceSession>(PerformanceSession(PerformanceSession::PerformanceTrackingGranularity::VeryGranular, "performance_log.json"));
	}
	else
	{
		performance_session = std::unique_ptr<PerformanceSession>(nullptr);
	}

	TRACE_FUNCTION(performance_session);

	RayBufferSize = camera.Resolution().X * camera.Resolution().Y;

	std::vector<GPUSphere> gpu_spheres;
	std::vector<GPUDiffuseMaterialParameters> diffuse_material_parameters;
	std::vector<GPUEmissiveMaterialParameters> emissive_material_parameters;
	std::vector<GPUVertex> gpu_vertices;
	std::vector<GPUFace> gpu_faces;
	ParseSceneData(scene, gpu_spheres, diffuse_material_parameters, emissive_material_parameters, gpu_vertices, gpu_faces);

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

	BufferData[(int)GPUBufferBindings::intersection_buffer].buffer_size = sizeof(GPUIntersection) * RayBufferSize;

	BufferData[(int)GPUBufferBindings::sphere_buffer].buffer_size = sizeof(GPUSphere) * gpu_spheres.size();
	BufferData[(int)GPUBufferBindings::sphere_buffer].data_pointer = &gpu_sphere_buffer;

	BufferData[(int)GPUBufferBindings::sample_buffer].buffer_size = sizeof(GPUSample) * RayBufferSize;

	BufferData[(int)GPUBufferBindings::colors_buffer].buffer_size = sizeof(GPUColor) * RayBufferSize;
	BufferData[(int)GPUBufferBindings::colors_buffer].data_pointer = &gpu_color_buffer;

	BufferData[(int)GPUBufferBindings::diffuse_material_parameters].buffer_size = sizeof(GPUDiffuseMaterialParameters) * diffuse_material_parameters.size();
	BufferData[(int)GPUBufferBindings::diffuse_material_parameters].data_pointer = &gpu_diffuse_material_parameters_buffer;
	
	BufferData[(int)GPUBufferBindings::emissive_material_parameters].buffer_size = sizeof(GPUEmissiveMaterialParameters) * emissive_material_parameters.size();
	BufferData[(int)GPUBufferBindings::emissive_material_parameters].data_pointer = &gpu_emissive_material_parameters_buffer;

	BufferData[(int)GPUBufferBindings::vertex_buffer].buffer_size = sizeof(GPUVertex) * gpu_vertices.size();
	BufferData[(int)GPUBufferBindings::vertex_buffer].data_pointer = &gpu_vertex_buffer;

	BufferData[(int)GPUBufferBindings::face_buffer].buffer_size = sizeof(GPUFace) * gpu_faces.size();
	BufferData[(int)GPUBufferBindings::face_buffer].data_pointer = &gpu_face_buffer;
	
	if (vk::Result::eSuccess != CreateAndMapMemories(ComputeQueueIndex))
	{
		throw std::exception("Failed to create and map memories for compute shader");
	}

	if (nullptr == gpu_sphere_buffer)
	{
		throw std::exception("Sphere buffer data for GPU compute failed to allocate");
	}

	if (nullptr == gpu_color_buffer)
	{
		throw std::exception("Color buffer data for GPU compute failed to allocate");
	}

	if (nullptr == gpu_diffuse_material_parameters_buffer)
	{
		throw std::exception("Diffuse material parameters for GPU compute failed to allocate");
	}

	if (nullptr == gpu_emissive_material_parameters_buffer)
	{
		throw std::exception("Emissive material parameters for GPU compute failed to allocate");
	}

	if (nullptr == gpu_vertex_buffer)
	{
		throw std::exception("Vertex buffer data for GPU compute failed to allocate");
	}

	if (nullptr == gpu_face_buffer)
	{
		throw std::exception("Face buffer data for GPU compute failed to allocate");
	}

	memcpy(gpu_sphere_buffer, gpu_spheres.data(), gpu_spheres.size() * sizeof(GPUSphere));
	memcpy(gpu_diffuse_material_parameters_buffer, diffuse_material_parameters.data(), diffuse_material_parameters.size() * sizeof(GPUDiffuseMaterialParameters));
	memcpy(gpu_emissive_material_parameters_buffer, emissive_material_parameters.data(), emissive_material_parameters.size() * sizeof(GPUEmissiveMaterialParameters));
	memcpy(gpu_vertex_buffer, gpu_vertices.data(), gpu_vertices.size() * sizeof(GPUVertex));
	memcpy(gpu_face_buffer, gpu_faces.data(), gpu_faces.size() * sizeof(GPUFace));
}

void GPURenderer::ParseSceneData(const IScene &scene, std::vector<GPUSphere> &out_gpu_spheres, std::vector<GPUDiffuseMaterialParameters> &out_diffuse_material_parameters, std::vector<GPUEmissiveMaterialParameters> &out_emissive_material_parameters, std::vector<GPUVertex> &out_gpu_vertices, std::vector<GPUFace> &out_gpu_faces)
{
	TRACE_FUNCTION(performance_session);

	using MaterialIndex = int32_t;
	struct GPUDiffuseMaterialParameterTracking
	{
		MaterialIndex material_index;
		GPUDiffuseMaterialParameters material_parameters;
	};

	struct GPUEmissiveMaterialParameterTracking
	{
		MaterialIndex material_index;
		GPUEmissiveMaterialParameters material_parameters;
	};

	std::unordered_map<const DiffuseBSDF *, GPUDiffuseMaterialParameterTracking> diffuse_materials;
	std::unordered_map<const EmissiveBSDF *, GPUEmissiveMaterialParameterTracking> emissive_materials;

	for (const auto &object : scene.Objects())
	{
		const std::shared_ptr<const IMaterial> material = object->Material();
		uint32_t material_id = -1;
		MaterialIndex material_index = 0;

		// TODO: Add more material types
		if (const DiffuseBSDF *const diffuse = dynamic_cast<const DiffuseBSDF *const>(material.get()))
		{
			material_id = static_cast<uint32_t>(GPURenderer::MaterialTypeID::diffuse);
			const auto &entry = diffuse_materials.find(diffuse);
			if (entry != diffuse_materials.end())
			{
				material_index = entry->second.material_index;
			}
			else
			{
				material_index = static_cast<MaterialIndex>(diffuse_materials.size());
				GPUDiffuseMaterialParameters new_parameters(diffuse);
				GPUDiffuseMaterialParameterTracking new_material_tracker
				{
					.material_index = static_cast<MaterialIndex>(material_index),
					.material_parameters = new_parameters
				};

				diffuse_materials.emplace(diffuse, new_material_tracker);
			}
		}
		else if (const EmissiveBSDF *const emissive = dynamic_cast<const EmissiveBSDF *const>(material.get()))
		{
			material_id = static_cast<uint32_t>(GPURenderer::MaterialTypeID::emissive);
			const auto &entry = emissive_materials.find(emissive);
			if (entry != emissive_materials.end())
			{
				material_index = entry->second.material_index;
			}
			else
			{
				material_index = static_cast<MaterialIndex>(emissive_materials.size());
				GPUEmissiveMaterialParameters new_parameters(emissive);
				GPUEmissiveMaterialParameterTracking new_material_tracker
				{
					.material_index = static_cast<MaterialIndex>(material_index),
					.material_parameters = new_parameters
				};

				emissive_materials.emplace(emissive, new_material_tracker);
			}
		}

		if (const Sphere *const sphere = dynamic_cast<const Sphere *const>(object))
		{
			out_gpu_spheres.emplace_back(sphere);
			GPUSphere &back = out_gpu_spheres.back();
			back.material_id = material_id;
			back.material_index = material_index;
		}
		else if (const Mesh *const mesh = dynamic_cast<const Mesh *const>(object))
		{
			for (const auto &vertex : mesh->VertexData)
			{
				out_gpu_vertices.emplace_back(vertex);
			}

			for (const auto &index : mesh->VertexIndices)
			{
				out_gpu_faces.emplace_back(index);
				GPUFace &back = out_gpu_faces.back();
				back.material_id = material_id;
				back.material_index = material_index;
			}
		}
	}

	out_diffuse_material_parameters.resize(diffuse_materials.size());
	for (const auto &kvp : diffuse_materials)
	{
		out_diffuse_material_parameters[kvp.second.material_index] = kvp.second.material_parameters;
	}

	out_emissive_material_parameters.resize(emissive_materials.size());
	for (const auto &kvp : emissive_materials)
	{
		out_emissive_material_parameters[kvp.second.material_index] = kvp.second.material_parameters;
	}

	// If there are no enties in a buffer add a dummy one so that the buffer will still exist
	// TODO: allow null buffers in shaders?

	if (out_diffuse_material_parameters.size() == 0)
	{
		out_diffuse_material_parameters.emplace_back();
	}

	if (out_emissive_material_parameters.size() == 0)
	{
		out_emissive_material_parameters.emplace_back();
	}

	if (out_gpu_spheres.size() == 0)
	{
		out_gpu_spheres.emplace_back();
	}
}

void *GPURenderer::CreateAndMapMemory(uint32_t queueFamilyIndex, GPURenderer::BufferCreationAndMappingData &buffer_data)
{
	TRACE_FUNCTION(performance_session);

	if (buffer_data.buffer_size == 0)
	{
		return nullptr;
	}

	vk::PhysicalDeviceMemoryProperties properties = {};
	PhysicalDevice.getMemoryProperties(&properties);

	// set memoryTypeIndex to an invalid entry in the properties.memoryTypes array
	uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;

	vk::MemoryPropertyFlags memory_flags = (nullptr == buffer_data.data_pointer) ? (vk::MemoryPropertyFlagBits::eDeviceLocal) : (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);

	for (uint32_t k = 0; k < properties.memoryTypeCount; k++)
	{
		if (((properties.memoryTypes[k].propertyFlags & memory_flags) == memory_flags) && (buffer_data.buffer_size < properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size))
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
	memoryAllocateInfo.allocationSize = buffer_data.buffer_size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	buffer_data.device_memory = device.allocateMemory(memoryAllocateInfo);

	vk::BufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.size = buffer_data.buffer_size;
	bufferCreateInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	bufferCreateInfo.queueFamilyIndexCount = 1;
	bufferCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;

	buffer_data.buffer = device.createBuffer(bufferCreateInfo);

	device.bindBufferMemory(buffer_data.buffer, buffer_data.device_memory, 0);
	
	if (nullptr != buffer_data.data_pointer)
	{
		return device.mapMemory(buffer_data.device_memory, 0, buffer_data.buffer_size);
	}

	return nullptr;
}

vk::Result GPURenderer::CreateAndMapMemories(uint32_t queueFamilyIndex)
{
	TRACE_FUNCTION(performance_session);

	vk::Result result = vk::Result::eSuccess;

	for (auto &buffer_data : BufferData)
	{
		void *data_pointer = CreateAndMapMemory(queueFamilyIndex, buffer_data);

		if (nullptr != buffer_data.data_pointer)
		{
			if (nullptr == data_pointer)
			{
				result = vk::Result::eErrorMemoryMapFailed;
			}

			*buffer_data.data_pointer = data_pointer;
		}
	}

	return result;
}

void GPURenderer::Render(std::shared_ptr<IImage> &out_image)
{
	TRACE_FUNCTION(performance_session);

	ElapsedTimer performance_timer;

	std::cout << "[RENDER STARTED]: line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << "\n";

	GPURayInitializer ray_initializer(device, ComputeQueueIndex, performance_session);
	GPURayIntersector ray_intersector(device, ComputeQueueIndex, performance_session);
	GPUMaterialCalculator material_calculator(device, ComputeQueueIndex, performance_session);
	GPUSampleAccumulator sample_accumulator(device, ComputeQueueIndex, performance_session);
	GPUSampleFinalizer sample_finalizer(device, ComputeQueueIndex, performance_session);

	for (size_t i = 0; i < samples; i++)
	{
		TRACE_SCOPE(performance_session, single_sample);

		std::cout << "############## CALCULATING SAMPLE " << i + 1 << " OF " << samples << " ##############\n";

		ray_initializer.Execute(camera, i, BufferData[(int)GPUBufferBindings::ray_buffer].buffer, BufferData[(int)GPUBufferBindings::intersection_buffer].buffer);

		bool ray_calculation_needed = true;
		size_t current_bounce_index = 0;

		while (current_bounce_index < max_bounces)
		{
			ray_intersector.Execute(RayBufferSize,
				BufferData[(int)GPUBufferBindings::ray_buffer].buffer,
				BufferData[(int)GPUBufferBindings::intersection_buffer].buffer,
				BufferData[(int)GPUBufferBindings::sphere_buffer].buffer,
				BufferData[(int)GPUBufferBindings::vertex_buffer].buffer, 
				BufferData[(int)GPUBufferBindings::face_buffer].buffer);

			material_calculator.Execute(RayBufferSize,
				BufferData[(int)GPUBufferBindings::intersection_buffer].buffer,
				BufferData[(int)GPUBufferBindings::ray_buffer].buffer,
				BufferData[(int)GPUBufferBindings::diffuse_material_parameters].buffer,
				BufferData[(int)GPUBufferBindings::emissive_material_parameters].buffer);

			current_bounce_index++;
		}

		sample_accumulator.Execute(RayBufferSize,
			BufferData[(int)GPUBufferBindings::intersection_buffer].buffer,
			BufferData[(int)GPUBufferBindings::sample_buffer].buffer);
	}

	sample_finalizer.Execute(RayBufferSize, 
		BufferData[(int)GPUBufferBindings::sample_buffer].buffer,
		BufferData[(int)GPUBufferBindings::colors_buffer].buffer, samples);

	std::cout << "[RENDER TIME]" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << "\n";

	Image *output_image = new Image(camera.Resolution());

	{
		TRACE_SCOPE(performance_session, image_creation);
		for (size_t y = 0; y < camera.Resolution().Y; y++)
		{
			size_t offset = y * camera.Resolution().X;
			for (size_t x = 0; x < camera.Resolution().X; x++)
			{
				const GPUColor &sample = ((GPUColor *)gpu_color_buffer)[offset + x];
				output_image->SetPixelColor(x, y, Color(sample.color[0],
					sample.color[1], sample.color[2], sample.color[3]));
			}
		}
	}

	out_image = std::shared_ptr<IImage>(output_image);

	std::cout << "[IMAGE CREATION TIME]" << ": line " << __LINE__ << ": time (ms): " << performance_timer.Poll().count() << "\n";
}

GPURenderer::~GPURenderer()
{
#if defined _DEBUG
	VKUtils::DestroyDebugMessenger(instance, debugMessenger);
#endif
}

