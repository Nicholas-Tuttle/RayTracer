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

GPURayIntersector::GPURayIntersector(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: world_intersector(device, compute_queue_index, session), sphere_intersector(device, compute_queue_index, session), mesh_intersector(device, compute_queue_index, session), performance_session(session)
{}

void GPURayIntersector::Execute(size_t incoming_ray_count, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_gpu_sphere_buffer, vk::Buffer input_vertex_buffer, vk::Buffer input_face_buffer)
{
	TRACE_FUNCTION(performance_session);

	// The world intersector should be first since everything will intersect with it and
	// it just sets up the max depth and starting color information in each intersection

	world_intersector.Execute(incoming_ray_count, input_gpu_ray_buffer, output_gpu_intersection_buffer);

	sphere_intersector.Execute(incoming_ray_count, input_gpu_ray_buffer, output_gpu_intersection_buffer, input_gpu_sphere_buffer);

	mesh_intersector.Execute(incoming_ray_count, input_gpu_ray_buffer, output_gpu_intersection_buffer, input_vertex_buffer, input_face_buffer);
}

#pragma endregion

#pragma region GPUWorldIntersector

GPURayIntersector::GPUWorldIntersector::GPUWorldIntersector(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: GPUComputeShader("GPUWorldIntersector.comp.spv", compute_queue_index, 2, 0, device, session), performance_session(session)
{}

void GPURayIntersector::GPUWorldIntersector::Execute(size_t incoming_ray_count, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::Execute(incoming_ray_count, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer});
}

#pragma endregion

#pragma region GPUSphereIntersector

GPURayIntersector::GPUSphereIntersector::GPUSphereIntersector(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: GPUComputeShader("GPUSphereIntersector.comp.spv", compute_queue_index, 3, 0, device, session), performance_session(session)
{}

void GPURayIntersector::GPUSphereIntersector::Execute(size_t incoming_ray_count,
	vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_gpu_sphere_buffer)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::Execute(incoming_ray_count, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer, input_gpu_sphere_buffer});
}

#pragma endregion

#pragma region GPUMeshIntersector

GPURayIntersector::GPUMeshIntersector::GPUMeshIntersector(vk::Device device, uint32_t compute_queue_index, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: GPUComputeShader("GPUMeshIntersector.comp.spv", compute_queue_index, 4, 0, device, session), performance_session(session)
{}

void GPURayIntersector::GPUMeshIntersector::Execute(size_t incoming_ray_count, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_vertex_buffer, vk::Buffer input_face_buffer)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::Execute(incoming_ray_count, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer, input_vertex_buffer, input_face_buffer});
}

#pragma endregion

