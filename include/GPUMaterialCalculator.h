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
		GPUMaterialCalculator(vk::Device device, PerformanceTracking::PerformanceSession *const session);
		void Execute(uint32_t compute_queue_index,
			size_t incoming_ray_count,
			vk::Buffer input_gpu_intersection_buffer,
			vk::Buffer output_gpu_ray_buffer,
			vk::Buffer input_diffuse_material_parameters,
			vk::Buffer input_emissive_material_parameters);

	private:
		vk::Device device;

		class GPUWorldMaterial : protected GPUComputeShader
		{
		public:
			GPUWorldMaterial(vk::Device device, PerformanceTracking::PerformanceSession *const session);
			void Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
				vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer);
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

			PerformanceTracking::PerformanceSession *performance_session;
		};

		class GPUDiffuseMaterial : protected GPUComputeShader
		{
		public:
			GPUDiffuseMaterial(vk::Device device, PerformanceTracking::PerformanceSession *const session);
			void Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
				vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer,
				vk::Buffer input_gpu_material_parameters);
		private:
			GPUDiffuseMaterial(const GPUDiffuseMaterial &other) = delete;
			GPUDiffuseMaterial(const GPUDiffuseMaterial &&other) = delete;

			struct push_constants
			{
				push_constants()
				{
					material_id = static_cast<uint32_t>(GPURenderer::MaterialTypeID::diffuse);
					random_seed = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				}

				uint32_t material_id;
				float random_seed;
			} DiffuseMaterialPushConstants;

			PerformanceTracking::PerformanceSession *performance_session;
		};

		class GPUEmissiveMaterial : protected GPUComputeShader
		{
		public:
			GPUEmissiveMaterial(vk::Device device, PerformanceTracking::PerformanceSession *const session);
			void Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
				vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer,
				vk::Buffer input_emissive_material_parameters);
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

			PerformanceTracking::PerformanceSession *performance_session;
		};

		GPUWorldMaterial world_material;
		GPUDiffuseMaterial diffuse_material;
		GPUEmissiveMaterial emissive_material;

		PerformanceTracking::PerformanceSession *performance_session;
	};
}
