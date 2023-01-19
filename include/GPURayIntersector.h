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
		void Execute(uint32_t ComputeQueueIndex, 
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
			void Execute(uint32_t ComputeQueueIndex,
				size_t incoming_ray_count,
				vk::Buffer input_gpu_ray_buffer,
				vk::Buffer output_gpu_intersection_buffer);
		private:
			vk::DescriptorSetLayout DescribeShader();
			vk::Result CreatePipeline();
			std::vector<vk::DescriptorSet> AllocateDescriptorSets();
			void UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet,
				vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer);
		};

		class GPUSphereIntersector : protected GPUComputeShader
		{
		public:
			GPUSphereIntersector(vk::Device device, const IScene &scene);
			void Execute(uint32_t ComputeQueueIndex,
				size_t incoming_ray_count,
				vk::Buffer input_gpu_ray_buffer,
				vk::Buffer output_gpu_intersection_buffer,
				vk::Buffer input_gpu_sphere_buffer);
		private:
			vk::DescriptorSetLayout DescribeShader();
			vk::Result CreatePipeline();
			std::vector<vk::DescriptorSet> AllocateDescriptorSets();
			void UpdateDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSet,
				vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_gpu_sphere_buffer);

			std::vector<GPUSphere> spheres;
		};

		GPUWorldIntersector world_intersector;
		GPUSphereIntersector sphere_intersector;
	};
}
