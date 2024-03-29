#include <GPURayInitializer.h>
#include "VulkanUtils.h"

using RayTracer::GPURayInitializer;
using RayTracer::Camera;
using RayTracer::VKUtils;
using RayTracer::PerformanceTracking::PerformanceSession;

const static size_t shader_local_size_x = 1024;

GPURayInitializer::GPURayInitializer(vk::Device device, uint32_t compute_queue_index, vk::Buffer output_gpu_ray_buffer, vk::Buffer output_gpu_intersection_buffer, const std::unique_ptr<PerformanceTracking::PerformanceSession> &session)
	: Device(device), CameraDataPushConstants(), GPUComputeShader("GPURayInitializer.comp.spv", compute_queue_index, 2, sizeof(GPURayInitializer::CameraDataPushConstants), device, std::vector<vk::Buffer>{output_gpu_ray_buffer, output_gpu_intersection_buffer}, session), performance_session(session)
{}

void GPURayInitializer::WriteCommandBuffers(const std::vector<std::reference_wrapper<vk::CommandBuffer>> &buffers, Camera camera, size_t seed)
{
	TRACE_FUNCTION(performance_session);

	if (RequiredCommandBuffers() != buffers.size())
	{
		return;
	}

	CameraDataPushConstants.camera_origin[0] = camera.Position().X;
	CameraDataPushConstants.camera_origin[1] = camera.Position().Y;
	CameraDataPushConstants.camera_origin[2] = camera.Position().Z;

	CameraDataPushConstants.forward_vector[0] = camera.ForwardVector().X;
	CameraDataPushConstants.forward_vector[1] = camera.ForwardVector().Y;
	CameraDataPushConstants.forward_vector[2] = camera.ForwardVector().Z;

	CameraDataPushConstants.right_vector[0] = camera.RightVector().X;
	CameraDataPushConstants.right_vector[1] = camera.RightVector().Y;
	CameraDataPushConstants.right_vector[2] = camera.RightVector().Z;

	CameraDataPushConstants.up_vector[0] = camera.UpVector().X;
	CameraDataPushConstants.up_vector[1] = camera.UpVector().Y;
	CameraDataPushConstants.up_vector[2] = camera.UpVector().Z;

	CameraDataPushConstants.focal_length_mm = static_cast<unsigned int>(camera.FocalLengthMM());
	CameraDataPushConstants.sensor_width_mm = static_cast<unsigned int>(camera.SensorWidthMM());
	CameraDataPushConstants.resolution_x = static_cast<unsigned int>(camera.Resolution().X);
	CameraDataPushConstants.resolution_y = static_cast<unsigned int>(camera.Resolution().Y);
	CameraDataPushConstants.seed = static_cast<unsigned int>(seed);

	GPUComputeShader::WriteCommandBuffer(buffers[0].get(), camera.Resolution().X * camera.Resolution().Y, static_cast<void *>(&CameraDataPushConstants));

	GPUComputeShader::WriteCalculationCompleteEvent(Device, buffers[buffers.size() - 1].get());
}
