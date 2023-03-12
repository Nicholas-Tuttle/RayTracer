#pragma once

#include "Camera.h"
#include "IScene.h"
#include "GPUStructs.h"
#include <vulkan/vulkan.hpp>
#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPURenderer
	{
	public:
		GPURenderer(const Camera &camera, unsigned int samples, const IScene &scene);
		~GPURenderer();

		void Render(std::shared_ptr<IImage> &out_image);
		bool GPUDebugEnabled = false;

		enum class MaterialTypeID
		{
			world,
			diffuse,
			emissive
		};
	private:
		vk::Instance instance = nullptr;
		vk::DebugUtilsMessengerEXT debugMessenger = nullptr;
		vk::PhysicalDevice PhysicalDevice = nullptr;
		vk::Device device = nullptr;
		uint32_t ComputeQueueIndex = 0;
		vk::DeviceSize RayBufferSize = 0;

		enum class GPUBufferBindings
		{
			ray_buffer,
			intersection_buffer,
			sphere_buffer,
			sample_buffer,
			diffuse_material_parameters,
			emissive_material_parameters,
			GPUBufferBindingCount
		};

		class BufferCreationAndMappingData
		{
		public:
			vk::DeviceSize buffer_size = 0;
			vk::BufferUsageFlagBits usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
			vk::DescriptorType descriptor_type = vk::DescriptorType::eStorageBuffer;
			vk::Buffer buffer;
			vk::DeviceMemory device_memory;
			void **data_pointer = nullptr;
			vk::MemoryPropertyFlags memory_property_bits;
		};

		Camera camera;
		unsigned int samples;
		const IScene &scene;

		std::vector<BufferCreationAndMappingData> BufferData;

		// Pointers are of type GPUSphere*
		void *gpu_sphere_buffer;
		// Pointers are of type GPUSample*
		void *gpu_sample_buffer;
		// Pointers are of type GPUDiffuseMaterialParameters*
		void *gpu_diffuse_material_parameters_buffer;
		// Pointers are of type GPUEmissiveMaterialParameters*
		void *gpu_emissive_material_parameters_buffer;
		
		void ParseSceneData(const IScene &scene, std::vector<GPUSphere> &out_gpu_spheres, std::vector<GPUDiffuseMaterialParameters> &out_diffuse_material_parameters, std::vector<GPUEmissiveMaterialParameters> &out_emissive_material_parameters);
		void *CreateAndMapMemory(uint32_t queueFamilyIndex, GPURenderer::BufferCreationAndMappingData &buffer_data);
		vk::Result CreateAndMapMemories(uint32_t queueFamilyIndex);

		PerformanceTracking::PerformanceSession *performance_session;
	};
}