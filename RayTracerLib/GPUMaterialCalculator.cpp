#include "GPUMaterialCalculator.h"

using RayTracer::GPUMaterialCalculator;

#pragma region Material Calculator

GPUMaterialCalculator::GPUMaterialCalculator(vk::Device device)
	: device(device), diffuse_material(device), world_material(device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << "\n";
#endif
}

void GPUMaterialCalculator::Execute(uint32_t compute_queue_index,
	size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer,
	vk::Buffer output_gpu_ray_buffer,
	vk::Buffer input_diffuse_material_parameters)
{
	world_material.Execute(compute_queue_index, incoming_ray_count, input_gpu_intersection_buffer, output_gpu_ray_buffer);
	diffuse_material.Execute(compute_queue_index, incoming_ray_count, input_gpu_intersection_buffer, output_gpu_ray_buffer, input_diffuse_material_parameters);
}

#pragma endregion

#pragma region World Material

GPUMaterialCalculator::GPUWorldMaterial::GPUWorldMaterial(vk::Device device)
	: WorldMaterialPushConstants(), GPUComputeShader("GPUWorldMaterial.comp.spv", 2, sizeof(GPUWorldMaterial::WorldMaterialPushConstants), device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << std::endl;
#endif
}

void GPUMaterialCalculator::GPUWorldMaterial::Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer)
{
	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer}, static_cast<void *>(&WorldMaterialPushConstants));
}

#pragma endregion

#pragma region Diffuse Material

GPUMaterialCalculator::GPUDiffuseMaterial::GPUDiffuseMaterial(vk::Device device)
	: DiffuseMaterialPushConstants(), GPUComputeShader("GPUDiffuseMaterial.comp.spv", 3, sizeof(GPUDiffuseMaterial::DiffuseMaterialPushConstants), device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << "\n";
#endif
}

void GPUMaterialCalculator::GPUDiffuseMaterial::Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_gpu_material_parameters)
{
	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer, input_gpu_material_parameters}, static_cast<void *>(&DiffuseMaterialPushConstants));
}

#pragma endregion

