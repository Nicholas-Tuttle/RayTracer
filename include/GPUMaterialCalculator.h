#pragma once

#include <vulkan/vulkan.hpp>
#include <iostream>
#include "GPUComputeShader.h"
#include "GPURenderer.h"

#include "PerformanceLogger.h"

namespace RayTracer
{
	class GPUMaterialCalculator
	{
	public:
		GPUMaterialCalculator(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
		void Execute(size_t incoming_ray_count, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_diffuse_material_parameters, vk::Buffer input_emissive_material_parameters);

	private:
		vk::Device device;

		class GPUWorldMaterial : protected GPUComputeShader
		{
		public:
			GPUWorldMaterial(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
			void Execute(size_t incoming_ray_count, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer);
		private:
			GPUWorldMaterial(const GPUWorldMaterial &other) = delete;
			GPUWorldMaterial(const GPUWorldMaterial &&other) = delete;

			struct push_constants
			{
				push_constants()
				{
					material_id = static_cast<uint32_t>(GPURenderer::MaterialTypeID::world);
				}

				uint32_t material_id;
			} WorldMaterialPushConstants;

			const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
		};

		class GPUDiffuseMaterial : protected GPUComputeShader
		{
		public:
			GPUDiffuseMaterial(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
			void Execute(size_t incoming_ray_count, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_gpu_material_parameters);
		private:
			GPUDiffuseMaterial(const GPUDiffuseMaterial &other) = delete;
			GPUDiffuseMaterial(const GPUDiffuseMaterial &&other) = delete;

			struct push_constants
			{
				push_constants()
				{
					material_id = static_cast<uint32_t>(GPURenderer::MaterialTypeID::diffuse);
					random_seed = static_cast<uint32_t>(rand());
				}

				uint32_t material_id;
				uint32_t random_seed;
			} DiffuseMaterialPushConstants;

			const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
		};

		class GPUEmissiveMaterial : protected GPUComputeShader
		{
		public:
			GPUEmissiveMaterial(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session);
			void Execute(size_t incoming_ray_count, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_emissive_material_parameters);
		private:
			GPUEmissiveMaterial(const GPUEmissiveMaterial &other) = delete;
			GPUEmissiveMaterial(const GPUEmissiveMaterial &&other) = delete;

			struct push_constants
			{
				push_constants()
				{
					material_id = static_cast<uint32_t>(GPURenderer::MaterialTypeID::emissive);
				}

				uint32_t material_id;
			} EmissiveMaterialPushConstants;

			const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
		};

		GPUWorldMaterial world_material;
		GPUDiffuseMaterial diffuse_material;
		GPUEmissiveMaterial emissive_material;

		const std::unique_ptr<PerformanceTracking::PerformanceSession> &performance_session;
	};
}
