#include "GPURayIntersector.h"
#include "VulkanUtils.h"
#include "ElapsedTimer.h"

using RayTracer::GPURayIntersector;
using RayTracer::IScene;
using RayTracer::Sphere;
using RayTracer::VKUtils;
using RayTracer::ElapsedTimer;
using RayTracer::PerformanceTracking::PerformanceSession;

const static size_t shader_local_size_x = 1024;

#pragma region GPURayIntersector

GPURayIntersector::GPURayIntersector(vk::Device device, const IScene &scene, PerformanceSession *const session)
	: world_intersector(device, session), sphere_intersector(device, scene, session), performance_session(session)
{}

void GPURayIntersector::Execute(uint32_t compute_queue_index,
	size_t incoming_ray_count,
	vk::Buffer input_gpu_ray_buffer,
	vk::Buffer output_gpu_intersection_buffer,
	vk::Buffer input_gpu_sphere_buffer)
{
	TRACE_FUNCTION(performance_session);

	// Right now this just does the world and sphere intersector, eventually 
	// a mesh intersector, volume intersector and others will be added

	// The world intersector should be first since everything will intersect with it and
	// it just sets up the max depth and starting color information in each intersection

	world_intersector.Execute(compute_queue_index, incoming_ray_count, input_gpu_ray_buffer, output_gpu_intersection_buffer);

	sphere_intersector.Execute(compute_queue_index, incoming_ray_count, input_gpu_ray_buffer, output_gpu_intersection_buffer, input_gpu_sphere_buffer);
}

#pragma endregion

#pragma region GPUWorldIntersector

GPURayIntersector::GPUWorldIntersector::GPUWorldIntersector(vk::Device device, PerformanceSession *const session)
	: GPUComputeShader("GPUWorldIntersector.comp.spv", 2, 0, device, session), performance_session(session)
{}

void GPURayIntersector::GPUWorldIntersector::Execute(uint32_t compute_queue_index,
	size_t incoming_ray_count,
	vk::Buffer input_gpu_ray_buffer,
	vk::Buffer output_gpu_intersection_buffer)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer});
}

#pragma endregion

#pragma region GPUSphereIntersector

GPURayIntersector::GPUSphereIntersector::GPUSphereIntersector(vk::Device device, const IScene &scene, PerformanceSession *const session)
	: GPUComputeShader("GPUSphereIntersector.comp.spv", 3, 0, device, session), performance_session(session)
{}

void GPURayIntersector::GPUSphereIntersector::Execute(uint32_t compute_queue_index, size_t incoming_ray_count,
	vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_gpu_sphere_buffer)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::Execute(compute_queue_index, incoming_ray_count, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer, input_gpu_sphere_buffer});
}

#pragma endregion
