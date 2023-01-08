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
		GPURendererV2(const Camera &camera, unsigned int samples);
		~GPURendererV2();
		void Render(const IScene &scene, std::shared_ptr<IImage> &out_image);
		bool GPUDebugEnabled = false;
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

		using gpu_buffer_vector = std::vector<void *>;

		class BufferCreationAndMappingData
		{
		public:
			vk::DeviceSize buffer_size = 0;
			vk::BufferUsageFlagBits usage_flag_bits = vk::BufferUsageFlagBits::eStorageBuffer;
			vk::DescriptorType descriptor_type = vk::DescriptorType::eStorageBuffer;
			std::vector<vk::Buffer> buffers;
			std::vector<vk::DeviceMemory> device_memories;
			gpu_buffer_vector *data_pointers = nullptr;

			void resize(size_t size)
			{
				buffers.resize(size, nullptr);
				device_memories.resize(size, nullptr);
				if (data_pointers)
				{
					data_pointers->resize(size, nullptr);
				}
			}
		};

		Camera camera;
		unsigned int samples;

		std::vector<BufferCreationAndMappingData> BufferData;

		// Pointers are of type GPURay*
		gpu_buffer_vector gpu_ray_buffers;
		// Pointers are of type GPUIntersection*
		gpu_buffer_vector gpu_intersection_buffers;
		
		void *CreateAndMapMemory(uint32_t queueFamilyIndex, const vk::DeviceSize memorySize, const vk::BufferUsageFlags usage_flags,
			vk::Buffer &vk_buffer, vk::DeviceMemory &device_memory);
		vk::Result CreateAndMapMemories(uint32_t queueFamilyIndex);
	};
}