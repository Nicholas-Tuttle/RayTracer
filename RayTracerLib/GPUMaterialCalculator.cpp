#include "GPUMaterialCalculator.h"

using RayTracer::GPUMaterialCalculator;

GPUMaterialCalculator::GPUMaterialCalculator(vk::Device device)
	: device(device), diffuse_material(device), world_material(device)
{
	std::cout << __FUNCTION__ << "\n";
}

void GPUMaterialCalculator::Execute(uint32_t compute_queue_index,
	size_t incoming_ray_count,
	vk::Buffer input_gpu_intersection_buffer,
	vk::Buffer output_gpu_ray_buffer)
{
	world_material.Execute(compute_queue_index, incoming_ray_count, input_gpu_intersection_buffer, output_gpu_ray_buffer);
	diffuse_material.Execute(compute_queue_index, incoming_ray_count, input_gpu_intersection_buffer, output_gpu_ray_buffer);
}
