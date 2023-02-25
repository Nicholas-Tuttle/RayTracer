#include "GPUWorldMaterial.h"
#include "VulkanUtils.h"

using RayTracer::GPUWorldMaterial;
using RayTracer::VKUtils;

const static size_t shader_local_size_x = 1024;

GPUWorldMaterial::GPUWorldMaterial(vk::Device device)
	: WorldMaterialPushConstants(), GPUComputeShader("GPUWorldMaterial.comp.spv", 2, sizeof(GPUWorldMaterial::WorldMaterialPushConstants), device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << std::endl;
#endif
}

void GPUWorldMaterial::Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer, vk::Buffer output_gpu_ray_buffer)
{
	WorldMaterialPushConstants.material_id = 0;

	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_intersection_buffer, output_gpu_ray_buffer}, static_cast<void *>(&WorldMaterialPushConstants));
}

