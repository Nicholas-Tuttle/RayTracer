#pragma once

#include "IScene.h"
#include "Sphere.h"
#include "GPUComputeShader.h"
#include "GPUStructs.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPURayIntersector
	{
	public:
		GPURayIntersector(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
		void Execute(size_t incoming_ray_count, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_gpu_sphere_buffer, vk::Buffer input_vertex_buffer, vk::Buffer input_face_buffer);
	private:
		GPURayIntersector(const GPURayIntersector &other) = delete;
		GPURayIntersector(const GPURayIntersector &&other) = delete;

		class GPUWorldIntersector : protected GPUComputeShader
		{
		public:
			GPUWorldIntersector(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
			void Execute(size_t incoming_ray_count, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer);
		private:
			const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
		};

		class GPUSphereIntersector : protected GPUComputeShader
		{
		public:
			GPUSphereIntersector(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
			void Execute(size_t incoming_ray_count, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_gpu_sphere_buffer);
		private:
			const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
		};

		class GPUMeshIntersector : protected GPUComputeShader
		{
		public:
			GPUMeshIntersector(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
			void Execute(size_t incoming_ray_count, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer,
				vk::Buffer input_vertex_buffer, vk::Buffer input_face_buffer);
		private:
			const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
		};

		GPUWorldIntersector world_intersector;
		GPUSphereIntersector sphere_intersector;
		GPUMeshIntersector mesh_intersector;

		const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
	};
}
