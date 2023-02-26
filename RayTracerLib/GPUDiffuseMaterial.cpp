#include "GPUDiffuseMaterial.h"
#include "VulkanUtils.h"

using RayTracer::GPUDiffuseMaterial;
using RayTracer::VKUtils;

const static size_t shader_local_size_x = 1024;

GPUDiffuseMaterial::GPUDiffuseMaterial(vk::Device device)
	: DiffuseMaterialPushConstants(), GPUComputeShader("GPUDiffuseMaterial.comp.spv", 3, sizeof(GPUDiffuseMaterial::DiffuseMaterialPushConstants), device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << "\n";
#endif
}

void GPUDiffuseMaterial::Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer, vk::Buffer input_gpu_material_parameters)
{
	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer, input_gpu_material_parameters}, static_cast<void *>(&DiffuseMaterialPushConstants));
}

