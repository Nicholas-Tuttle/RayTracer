#include "GPUMaterialCalculator.h"
#include "ElapsedTimer.h"

using RayTracer::GPUMaterialCalculator;
using RayTracer::ElapsedTimer;
using RayTracer::PerformanceTracking::PerformanceSession;

#pragma region Material Calculator

GPUMaterialCalculator::GPUMaterialCalculator(vk::Device device, uint32_t compute_queue_index, vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_diffuse_material_parameters, vk::Buffer input_emissive_material_parameters, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: Device(device), diffuse_material(device, compute_queue_index, input_gpu_intersection_buffer, output_gpu_ray_buffer, input_diffuse_material_parameters, session), world_material(device, compute_queue_index, session, input_gpu_intersection_buffer, output_gpu_ray_buffer), emissive_material(device, compute_queue_index, input_gpu_intersection_buffer, output_gpu_ray_buffer, input_emissive_material_parameters, session), performance_session(session)
{}

void GPUMaterialCalculator::WriteCommandBuffers(std::vector<std::reference_wrapper<vk::CommandBuffer>> buffers, size_t incoming_ray_count)
{
	TRACE_FUNCTION(performance_session);

	if (RequiredCommandBuffers() != buffers.size())
	{
		return;
	}

	world_material.WriteCommandBuffer(buffers[0].get(), incoming_ray_count);

	diffuse_material.WriteCommandBuffer(buffers[1].get(), incoming_ray_count);

	emissive_material.WriteCommandBuffer(buffers[2].get(), incoming_ray_count);

	GPUComputeShader::WriteCalculationCompleteEvent(Device, buffers[buffers.size() - 1].get());
}

#pragma endregion

#pragma region World Material

GPUMaterialCalculator::GPUWorldMaterial::GPUWorldMaterial(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer)
	: WorldMaterialPushConstants(), GPUComputeShader("GPUWorldMaterial.comp.spv", compute_queue_index, 2, sizeof(GPUWorldMaterial::WorldMaterialPushConstants), device, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer}, session), performance_session(session)
{}

void GPUMaterialCalculator::GPUWorldMaterial::WriteCommandBuffer(vk::CommandBuffer &buffer, size_t incoming_ray_count)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::WriteCommandBuffer(buffer, incoming_ray_count, static_cast<void *>(&WorldMaterialPushConstants));
}

#pragma endregion

#pragma region Diffuse Material

GPUMaterialCalculator::GPUDiffuseMaterial::GPUDiffuseMaterial(vk::Device device, uint32_t compute_queue_index,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_gpu_material_parameters, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: DiffuseMaterialPushConstants(), GPUComputeShader("GPUDiffuseMaterial.comp.spv", compute_queue_index, 3, sizeof(GPUDiffuseMaterial::DiffuseMaterialPushConstants), device, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer, input_gpu_material_parameters}, session), performance_session(session)
{}

void GPUMaterialCalculator::GPUDiffuseMaterial::WriteCommandBuffer(vk::CommandBuffer &buffer, size_t incoming_ray_count)
{
	TRACE_FUNCTION(performance_session);

	DiffuseMaterialPushConstants.random_seed = static_cast<uint32_t>(rand());

	GPUComputeShader::WriteCommandBuffer(buffer, incoming_ray_count, static_cast<void *>(&DiffuseMaterialPushConstants));
}

#pragma endregion

#pragma region Emissive Material

GPUMaterialCalculator::GPUEmissiveMaterial::GPUEmissiveMaterial(vk::Device device, uint32_t compute_queue_index,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_emissive_material_parameters, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: EmissiveMaterialPushConstants(), GPUComputeShader("GPUEmissiveMaterial.comp.spv", compute_queue_index, 3, sizeof(GPUEmissiveMaterial::EmissiveMaterialPushConstants), device, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer, input_emissive_material_parameters}, session), performance_session(session)
{}

void GPUMaterialCalculator::GPUEmissiveMaterial::WriteCommandBuffer(vk::CommandBuffer &buffer, size_t incoming_ray_count)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::WriteCommandBuffer(buffer, incoming_ray_count, static_cast<void *>(&EmissiveMaterialPushConstants));
}

#pragma endregion

