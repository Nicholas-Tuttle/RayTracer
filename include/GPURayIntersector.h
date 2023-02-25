#pragma once

#include "IScene.h"
#include "Sphere.h"
#include "GPUComputeShader.h"
#include "GPUStructs.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

namespace RayTracer
{
	class GPURayIntersector
	{
	public:
		GPURayIntersector(vk::Device device, const IScene &scene);
		void Execute(uint32_t compute_queue_index,
			size_t incoming_ray_count,
			vk::Buffer input_gpu_ray_buffer, 
			vk::Buffer output_gpu_intersection_buffer,
			vk::Buffer input_gpu_sphere_buffer);
	private:
		GPURayIntersector(const GPURayIntersector &other) = delete;
		GPURayIntersector(const GPURayIntersector &&other) = delete;

		class GPUWorldIntersector : protected GPUComputeShader
		{
		public:
			GPUWorldIntersector(vk::Device device);
			void Execute(uint32_t compute_queue_index,
				size_t incoming_ray_count,
				vk::Buffer input_gpu_ray_buffer,
				vk::Buffer output_gpu_intersection_buffer);
		};

		class GPUSphereIntersector : protected GPUComputeShader
		{
		public:
			GPUSphereIntersector(vk::Device device, const IScene &scene);
			void Execute(uint32_t compute_queue_index,
				size_t incoming_ray_count,
				vk::Buffer input_gpu_ray_buffer,
				vk::Buffer output_gpu_intersection_buffer,
				vk::Buffer input_gpu_sphere_buffer);
		};

		GPUWorldIntersector world_intersector;
		GPUSphereIntersector sphere_intersector;
	};
}
