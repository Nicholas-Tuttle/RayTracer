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
		struct GPURendererInitParameters
		{
			const Camera &camera;
			unsigned int samples;
			size_t max_bounces;
			const IScene &scene;
			bool trace_performance;

			GPURendererInitParameters(const Camera &camera, const IScene &scene)
				: camera(camera), scene(scene), samples(1), max_bounces(8), trace_performance(false)
			{}
		};

		GPURenderer(const GPURendererInitParameters &params);
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
			colors_buffer,
			diffuse_material_parameters,
			emissive_material_parameters,
			GPUBufferBindingCount
		};

		class BufferCreationAndMappingData
		{
		public:
			vk::DeviceSize buffer_size = 0;
			vk::Buffer buffer = VK_NULL_HANDLE;
			vk::DeviceMemory device_memory = VK_NULL_HANDLE;
			void **data_pointer = nullptr;
		};

		Camera camera;
		unsigned int samples;
		size_t max_bounces;
		const IScene &scene;

		std::vector<BufferCreationAndMappingData> BufferData;

		// Pointers are of type GPUSphere*
		void *gpu_sphere_buffer;
		// Pointers are of type GPUColor*
		void *gpu_color_buffer;
		// Pointers are of type GPUDiffuseMaterialParameters*
		void *gpu_diffuse_material_parameters_buffer;
		// Pointers are of type GPUEmissiveMaterialParameters*
		void *gpu_emissive_material_parameters_buffer;
		
		void ParseSceneData(const IScene &scene, std::vector<GPUSphere> &out_gpu_spheres, std::vector<GPUDiffuseMaterialParameters> &out_diffuse_material_parameters, std::vector<GPUEmissiveMaterialParameters> &out_emissive_material_parameters);
		void *CreateAndMapMemory(uint32_t queueFamilyIndex, GPURenderer::BufferCreationAndMappingData &buffer_data);
		vk::Result CreateAndMapMemories(uint32_t queueFamilyIndex);

		std::unique_ptr<PerformanceTracking::PerformanceSession> performance_session;
	};
}