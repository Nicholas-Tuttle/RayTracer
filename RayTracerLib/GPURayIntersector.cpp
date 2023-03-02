#include "GPURayIntersector.h"
#include "VulkanUtils.h"
#include "ElapsedTimer.h"

using RayTracer::GPURayIntersector;
using RayTracer::GPURay;
using RayTracer::IScene;
using RayTracer::Sphere;
using RayTracer::VKUtils;
using RayTracer::ElapsedTimer;

const static size_t shader_local_size_x = 1024;

#pragma region GPURayIntersector

GPURayIntersector::GPURayIntersector(vk::Device device, const IScene &scene)
	: world_intersector(device), sphere_intersector(device, scene)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << std::endl;
#endif
}

void GPURayIntersector::Execute(uint32_t compute_queue_index,
	size_t incoming_ray_count,
	vk::Buffer input_gpu_ray_buffer,
	vk::Buffer output_gpu_intersection_buffer,
	vk::Buffer input_gpu_sphere_buffer)
{
	// Right now this just does the world and sphere intersector, eventually 
	// a mesh intersector, volume intersector and others will be added

	// The world intersector should be first since everything will intersect with it and
	// it just sets up the max depth and starting color information in each intersection

	ElapsedTimer timer;

	world_intersector.Execute(compute_queue_index, incoming_ray_count, input_gpu_ray_buffer, output_gpu_intersection_buffer);

	std::cout << "\t\t[World Intersection] time (ms): " << timer.Poll().count() << "\n";

	sphere_intersector.Execute(compute_queue_index, incoming_ray_count, input_gpu_ray_buffer, output_gpu_intersection_buffer, input_gpu_sphere_buffer);

	std::cout << "\t\t[Sphere Intersection] time (ms): " << timer.Poll().count() << "\n";
}

#pragma endregion

#pragma region GPUWorldIntersector

GPURayIntersector::GPUWorldIntersector::GPUWorldIntersector(vk::Device device)
	: GPUComputeShader("GPUWorldIntersector.comp.spv", 2, 0, device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << std::endl;
#endif
}

void GPURayIntersector::GPUWorldIntersector::Execute(uint32_t compute_queue_index,
	size_t incoming_ray_count,
	vk::Buffer input_gpu_ray_buffer,
	vk::Buffer output_gpu_intersection_buffer)
{
	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer});
}

#pragma endregion

#pragma region GPUSphereIntersector

GPURayIntersector::GPUSphereIntersector::GPUSphereIntersector(vk::Device device, const IScene &scene)
	: GPUComputeShader("GPUSphereIntersector.comp.spv", 3, 0, device)
{
#ifdef _DEBUG
	std::cout << __FUNCTION__ << std::endl;
#endif
}

void GPURayIntersector::GPUSphereIntersector::Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
	vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_gpu_sphere_buffer)
{
	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer, input_gpu_sphere_buffer});
}

#pragma endregion
