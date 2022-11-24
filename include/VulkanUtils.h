#pragma once

#include <vulkan/vulkan.hpp>

namespace RayTracer
{
    class VKUtils
    {
    public:
        static const std::vector<const char*> DebugInstanceLayers;
        static const std::vector<const char*> DebugInstanceExtensions;
        static vk::ValidationFeaturesEXT VulkanDebugPrintfInstanceItem;
        
        static VKAPI_ATTR VkBool32 VKAPI_CALL DefaultVulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

        // Verifies the instance layers in requiredInstanceLayers are available
        static bool VerifyInstanceLayers(const std::vector<const char *> &InstanceExtensions);

        // Verifies the instance extensions in requiredInstanceExtensions are available
        static bool VerifyInstanceExtensions(const std::vector<const char *> &requiredInstanceLayers);

        static vk::Instance CreateHeadlessVulkanInstance(const std::vector<const char *> &InstanceLayers,
            const std::vector<const char *> &InstanceExtensions, const void *instance_items);

        // Creates a Vulkan Debug Messenger that receives all messages
        static vk::DebugUtilsMessengerEXT CreateDebugMessenger(vk::Instance &instance, PFN_vkDebugUtilsMessengerCallbackEXT userCallback);
        static void DestroyDebugMessenger(vk::Instance &instance, vk::DebugUtilsMessengerEXT debugUtilsMessenger);

        static void EnumerateDevices(vk::Instance instance, std::vector<vk::PhysicalDevice> &out_devices);

        static vk::Result GetBestComputeQueue(vk::PhysicalDevice physicalDevice, uint32_t &queueFamilyIndex);

        static vk::Device CreateDevice(vk::PhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

        static std::vector<uint8_t> ReadShaderFileToBytes(const std::string &filename);
    };
}

