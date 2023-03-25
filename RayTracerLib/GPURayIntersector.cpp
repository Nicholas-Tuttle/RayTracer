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

GPURayIntersector::GPURayIntersector(vk::Device device, uint32_t compute_queue_index, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_gpu_sphere_buffer, vk::Buffer input_vertex_buffer, vk::Buffer input_face_buffer, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: Device(device), world_intersector(device, compute_queue_index, input_gpu_ray_buffer, output_gpu_intersection_buffer, session), sphere_intersector(device, compute_queue_index, input_gpu_ray_buffer, output_gpu_intersection_buffer, input_gpu_sphere_buffer, session), mesh_intersector(device, compute_queue_index, input_gpu_ray_buffer, output_gpu_intersection_buffer, input_vertex_buffer, input_face_buffer, session), performance_session(session)
{}

void GPURayIntersector::WriteCommandBuffers(const std::vector<std::reference_wrapper<vk::CommandBuffer>> &buffers, size_t incoming_ray_count)
{
	TRACE_FUNCTION(performance_session);

	if (RequiredCommandBuffers() != buffers.size())
	{
		return;
	}

	// The world intersector should be first since everything will intersect with it and
	// it just sets up the max depth and starting color information in each intersection

	world_intersector.WriteCommandBuffer(buffers[0].get(), incoming_ray_count);

	sphere_intersector.WriteCommandBuffer(buffers[1].get(), incoming_ray_count);

	mesh_intersector.WriteCommandBuffer(buffers[2].get(), incoming_ray_count);

	GPUComputeShader::WriteCalculationCompleteEvent(Device, buffers[buffers.size() - 1].get());
}

#pragma endregion

#pragma region GPUWorldIntersector

GPURayIntersector::GPUWorldIntersector::GPUWorldIntersector(vk::Device device, uint32_t compute_queue_index, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: GPUComputeShader("GPUWorldIntersector.comp.spv", compute_queue_index, 2, 0, device, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer}, session), performance_session(session)
{}

void GPURayIntersector::GPUWorldIntersector::WriteCommandBuffer(vk::CommandBuffer &buffer, size_t incoming_ray_count)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::WriteCommandBuffer(buffer, incoming_ray_count);
}

#pragma endregion

#pragma region GPUSphereIntersector

GPURayIntersector::GPUSphereIntersector::GPUSphereIntersector(vk::Device device, uint32_t compute_queue_index,
	vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_gpu_sphere_buffer, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: GPUComputeShader("GPUSphereIntersector.comp.spv", compute_queue_index, 3, 0, device, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer, input_gpu_sphere_buffer}, session), performance_session(session)
{}

void GPURayIntersector::GPUSphereIntersector::WriteCommandBuffer(vk::CommandBuffer &buffer, size_t incoming_ray_count)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::WriteCommandBuffer(buffer, incoming_ray_count);
}

#pragma endregion

#pragma region GPUMeshIntersector

GPURayIntersector::GPUMeshIntersector::GPUMeshIntersector(vk::Device device, uint32_t compute_queue_index, vk::Buffer input_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, vk::Buffer input_vertex_buffer, vk::Buffer input_face_buffer, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: GPUComputeShader("GPUMeshIntersector.comp.spv", compute_queue_index, 4, 0, device, std::vector<vk::Buffer>{input_gpu_ray_buffer, output_gpu_intersection_buffer, input_vertex_buffer, input_face_buffer}, session), performance_session(session)
{}

void GPURayIntersector::GPUMeshIntersector::WriteCommandBuffer(vk::CommandBuffer &buffer, size_t incoming_ray_count)
{
	TRACE_FUNCTION(performance_session);

	GPUComputeShader::WriteCommandBuffer(buffer, incoming_ray_count);
}

#pragma endregion

