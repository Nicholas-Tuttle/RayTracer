#pragma once
#include <vulkan/vulkan.h>

#include "IImage.h"
#include "IScene.h"
#include "Camera.h"

namespace RayTracer
{
	class GPURenderer
	{
	public:
		void Render(const Camera& camera, unsigned int samples, const std::unique_ptr<IScene> &scene, std::unique_ptr<IImage> &out_image);
		bool GPUDebugEnabled;

		GPURenderer() : GPUDebugEnabled(false) {}
	private:
		VkResult CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkDevice &device);
		VkResult GetBestComputeQueue(VkPhysicalDevice physicalDevice, uint32_t &queueFamilyIndex);
		VkResult EnumerateDevices(VkInstance instance, VkPhysicalDevice *&devices, uint32_t &device_count);
		VkResult CreateHeadlessVulkanInstance(VkInstance &instance);
		void DestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger);
		VkResult CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT *debugMessenger);
		bool VerifyInstanceExtensions();
		bool VerifyInstanceLayers();

		std::vector<const char *> requiredInstanceLayers = {};
		std::vector<const char *> requiredInstanceExtensions = {};
	};
}

