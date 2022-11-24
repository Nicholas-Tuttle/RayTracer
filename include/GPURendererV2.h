#pragma once

#include "Camera.h"
#include "IScene.h"
#include "GPUStructs.h"
#include <vulkan/vulkan.hpp>

namespace RayTracer
{
	class GPURendererV2
	{
	public:
		GPURendererV2(const Camera &camera);
		~GPURendererV2();
		void Render(unsigned int samples, const IScene &scene, std::shared_ptr<IImage> &out_image);
		bool GPUDebugEnabled = false;

#if defined VK_USE_PLATFORM_WIN32_KHR
		GPURendererV2(const Camera &camera, HINSTANCE hInstance);
#endif
	private:
		vk::Instance instance = nullptr;
		vk::DebugUtilsMessengerEXT debugMessenger = nullptr;
		vk::PhysicalDevice PhysicalDevice = nullptr;
		vk::Device device = nullptr;
		uint32_t ComputeQueueIndex = 0;

		enum class GPUBufferBindings
		{
			ray_buffer,
			intersection_buffer,
			GPUBufferBindingCount
		};

		struct BufferCreationAndMappingData
		{
			vk::DeviceSize buffer_size;
			vk::Buffer buffer;
			vk::DeviceMemory device_memory;
			vk::BufferUsageFlagBits usage_flag_bits;
			vk::DescriptorType descriptor_type;
			void **data_pointer;
		};

		Camera camera;

		std::vector<BufferCreationAndMappingData> BufferData;

		GPURay *gpu_ray_buffer = nullptr;
		GPUIntersection *gpu_intersection_buffer = nullptr;
		
		void *CreateAndMapMemory(uint32_t queueFamilyIndex, const vk::DeviceSize memorySize, const vk::BufferUsageFlags usage_flags,
			vk::Buffer &vk_buffer, vk::DeviceMemory &device_memory);
		vk::Result CreateAndMapMemories(uint32_t queueFamilyIndex);
	};
}