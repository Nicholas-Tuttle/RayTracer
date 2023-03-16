#include "GPUMaterialCalculator.h"
#include "ElapsedTimer.h"

using RayTracer::GPUMaterialCalculator;
using RayTracer::ElapsedTimer;
using RayTracer::PerformanceTracking::PerformanceSession;

#pragma region Material Calculator

GPUMaterialCalculator::GPUMaterialCalculator(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: device(device), diffuse_material(device, compute_queue_index, session), world_material(device, compute_queue_index, session), emissive_material(device, compute_queue_index, session), performance_session(session)
{}

void GPUMaterialCalculator::Execute(size_t incoming_ray_count, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_diffuse_material_parameters, vk::Buffer input_emissive_material_parameters)
{
	TRACE_FUNCTION(performance_session);

	world_material.Execute(incoming_ray_count, input_gpu_intersection_buffer, output_gpu_ray_buffer);

	diffuse_material.Execute(incoming_ray_count, input_gpu_intersection_buffer, output_gpu_ray_buffer, input_diffuse_material_parameters);

	emissive_material.Execute(incoming_ray_count, input_gpu_intersection_buffer, output_gpu_ray_buffer,
		input_emissive_material_parameters);
}

#pragma endregion

#pragma region World Material

GPUMaterialCalculator::GPUWorldMaterial::GPUWorldMaterial(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: WorldMaterialPushConstants(), GPUComputeShader("GPUWorldMaterial.comp.spv", compute_queue_index, 2, sizeof(GPUWorldMaterial::WorldMaterialPushConstants), device, session), performance_session(session)
{}

void GPUMaterialCalculator::GPUWorldMaterial::Execute(size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::Execute(incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer}, static_cast<void *>(&WorldMaterialPushConstants));
}

#pragma endregion

#pragma region Diffuse Material

GPUMaterialCalculator::GPUDiffuseMaterial::GPUDiffuseMaterial(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: DiffuseMaterialPushConstants(), GPUComputeShader("GPUDiffuseMaterial.comp.spv", compute_queue_index, 3, sizeof(GPUDiffuseMaterial::DiffuseMaterialPushConstants), device, session), performance_session(session)
{}

void GPUMaterialCalculator::GPUDiffuseMaterial::Execute(size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_gpu_material_parameters)
{
	TRACE_FUNCTION(performance_session);

	DiffuseMaterialPushConstants.random_seed = static_cast<uint32_t>(rand());

	GPUComputeShader::Execute(incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer, input_gpu_material_parameters}, static_cast<void *>(&DiffuseMaterialPushConstants));
}

#pragma endregion

#pragma region Emissive Material

GPUMaterialCalculator::GPUEmissiveMaterial::GPUEmissiveMaterial(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: EmissiveMaterialPushConstants(), GPUComputeShader("GPUEmissiveMaterial.comp.spv", compute_queue_index, 3, sizeof(GPUEmissiveMaterial::EmissiveMaterialPushConstants), device, session), performance_session(session)
{}

void GPUMaterialCalculator::GPUEmissiveMaterial::Execute(size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_emissive_material_parameters)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::Execute(incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer, input_emissive_material_parameters}, static_cast<void *>(&EmissiveMaterialPushConstants));
}

#pragma endregion

