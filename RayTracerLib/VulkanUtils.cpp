#include "VulkanUtils.h"
#include "vulkan/vulkan.hpp"
#include <iostream>
#include <fstream>

using RayTracer::VKUtils;

#ifdef _DEBUG
const std::vector<const char*> VKUtils::DebugInstanceLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> VKUtils::DebugInstanceExtensions = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

const std::vector<const char *> VKUtils::DebugDeviceExtensions = {
    VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
};
#else
const std::vector<const char *> VKUtils::DebugInstanceLayers = {};
const std::vector<const char *> VKUtils::DebugInstanceExtensions = {};
const std::vector<const char *> VKUtils::DebugDeviceExtensions = {};
#endif

static vk::ValidationFeatureEnableEXT VulkanDebugPrintfInstanceItemEnable = vk::ValidationFeatureEnableEXT::eDebugPrintf;
vk::ValidationFeaturesEXT VKUtils::VulkanDebugPrintfInstanceItem(1, &VulkanDebugPrintfInstanceItemEnable, 0, nullptr);

// This Vulkan debug callback receives messages from the debugPrintfEXT (GLSL) or printf (HLSL) functions in the
// compute shaders, along with other vulkan messages. 
// For more reference, see:
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerEXT.html
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCreateDebugUtilsMessengerEXT.html
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkDestroyDebugUtilsMessengerEXT.html
// for more info.
// NOTE: This default callback will take the portion of the string after the last '|' character and print
// that due to the default formatting of Vulkan callback messages containing a lot of extra information
VKAPI_ATTR VkBool32 VKAPI_CALL VKUtils::DefaultVulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData
)
{
#ifdef _DEBUG
    std::cout << "[VULKAN DEBUG] : ";
    std::string full_message(pCallbackData->pMessage);
    std::string user_message = full_message.substr(full_message.find_last_of('|') + 1);
    std::cout << user_message << "\n";
#endif
    return VK_FALSE;
}

bool VKUtils::VerifyInstanceLayers(const std::vector<const char *> &requiredInstanceLayers)
{
    if (requiredInstanceLayers.size() == 0)
    {
        return true;
    }

    vk::Result vulkan_result = vk::Result::eSuccess;

    uint32_t layerCount = 0;
    vulkan_result = vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
    if (vk::Result::eSuccess != vulkan_result)
    {
        return false;
    }

    std::vector<vk::LayerProperties> availableLayers(layerCount);
    vulkan_result = vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    if (vk::Result::eSuccess != vulkan_result)
    {
        return false;
    }

    for (const char *layerName : requiredInstanceLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

bool VKUtils::VerifyInstanceExtensions(const std::vector<const char *> &requiredInstanceExtensions)
{
    if (requiredInstanceExtensions.size() == 0)
    {
        return true;
    }

    vk::Result vulkan_result = vk::Result::eSuccess;

    uint32_t extensionCount = 0;
    vulkan_result = vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (vk::Result::eSuccess != vulkan_result)
    {
        return false;
    }
    
    std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
    vulkan_result = vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
    if (vk::Result::eSuccess != vulkan_result)
    {
        return false;
    }

    for (const char *extensionName : requiredInstanceExtensions)
    {
        bool layerFound = false;

        for (const auto &extensionProperties : availableExtensions)
        {
            if (strcmp(extensionName, extensionProperties.extensionName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

bool VKUtils::VerifyDeviceExtensions(vk::PhysicalDevice device, const std::vector<const char *> &requiredDeviceExtensions)
{
    if (requiredDeviceExtensions.size() == 0)
    {
        return true;
    }

    std::vector<vk::ExtensionProperties> extensions = device.enumerateDeviceExtensionProperties();

    for (const char *extensionName : requiredDeviceExtensions)
    {
        bool layerFound = false;

        for (const auto &extensionProperties : extensions)
        {
            if (strcmp(extensionName, extensionProperties.extensionName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

vk::Instance VKUtils::CreateHeadlessVulkanInstance(const std::vector<const char *>& InstanceLayers,
    const std::vector<const char *> &InstanceExtensions, const void *instance_items)
{
    vk::ApplicationInfo applicationInfo;

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(InstanceLayers.size());
    createInfo.ppEnabledLayerNames = InstanceLayers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = InstanceExtensions.data();
    createInfo.pNext = instance_items;

    return vk::createInstance(createInfo);
}

vk::DebugUtilsMessengerEXT VKUtils::CreateDebugMessenger(vk::Instance &instance, PFN_vkDebugUtilsMessengerCallbackEXT userCallback)
{
    //TODO: This seems wrong switching between the C++ and C API
    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    createInfo.pfnUserCallback = userCallback;

    VkDebugUtilsMessengerCreateInfoEXT create_info_c = {};
    create_info_c.sType = (VkStructureType)createInfo.sType;
    create_info_c.messageSeverity = (VkDebugUtilsMessageSeverityFlagsEXT)createInfo.messageSeverity;
    create_info_c.messageType = (VkDebugUtilsMessageTypeFlagsEXT)createInfo.messageType;
    create_info_c.pfnUserCallback = createInfo.pfnUserCallback;

    auto function = (PFN_vkCreateDebugUtilsMessengerEXT)instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
    if (nullptr == function)
    {
        return vk::DebugUtilsMessengerEXT();
    }

    VkDebugUtilsMessengerEXT debug_utils_messenger = { 0 };
    function(instance, &create_info_c, nullptr, &debug_utils_messenger);
    vk::DebugUtilsMessengerEXT debugUtilsMessenger(debug_utils_messenger);
    return debugUtilsMessenger;
}

void VKUtils::DestroyDebugMessenger(vk::Instance &instance, vk::DebugUtilsMessengerEXT debugUtilsMessenger)
{
    auto function = (PFN_vkDestroyDebugUtilsMessengerEXT)instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT");
    if (nullptr == function)
    {
        return;
    }

    function(instance, debugUtilsMessenger, nullptr);
}

void VKUtils::EnumerateDevices(vk::Instance instance, std::vector<vk::PhysicalDevice> &out_devices)
{
    out_devices = instance.enumeratePhysicalDevices();
}

vk::Result VKUtils::GetBestComputeQueue(vk::PhysicalDevice physicalDevice, uint32_t &queueFamilyIndex)
{
    uint32_t queueFamilyPropertiesCount = 0;
    physicalDevice.getQueueFamilyProperties(&queueFamilyPropertiesCount, nullptr);

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
    physicalDevice.getQueueFamilyProperties(&queueFamilyPropertiesCount, queueFamilyProperties.data());
    
    // first try and find a queue that has just the compute bit set
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++)
    {
        // mask out the sparse binding bit and the transfer bit
        const vk::Flags<vk::QueueFlagBits> maskedFlags = (~(vk::QueueFlagBits::eTransfer | vk::QueueFlagBits::eSparseBinding) 
            & queueFamilyProperties[i].queueFlags);

        if (!(vk::QueueFlagBits::eGraphics & maskedFlags) && (vk::QueueFlagBits::eCompute & maskedFlags))
        {
            queueFamilyIndex = i;
            return vk::Result::eSuccess;
        }
    }

    // lastly get any queue that'll work for us
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++)
    {
        // mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
        const vk::Flags<vk::QueueFlagBits> maskedFlags = (~(vk::QueueFlagBits::eTransfer | vk::QueueFlagBits::eSparseBinding) 
            & queueFamilyProperties[i].queueFlags);

        if (vk::QueueFlagBits::eCompute & maskedFlags)
        {
            queueFamilyIndex = i;
            return vk::Result::eSuccess;
        }
    }

    return vk::Result::eErrorInitializationFailed;
}

vk::Device VKUtils::CreateDevice(vk::PhysicalDevice physicalDevice, const std::vector<const char *> &DeviceEntensions, uint32_t queueFamilyIndex)
{
    const float queuePrioritory = 1.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &queuePrioritory;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceEntensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = DeviceEntensions.data();

    return physicalDevice.createDevice(deviceCreateInfo);
}

std::vector<uint8_t> VKUtils::ReadShaderFileToBytes(const std::string &filename)
{
    std::ifstream file;
    file.open(filename, std::ifstream::in | std::ifstream::ate | std::ifstream::binary);

    if (!file.is_open() || !file.good()) {
        return std::vector<uint8_t>();
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<uint8_t> buffer(fileSize);
    file.seekg(0);
    file.read(reinterpret_cast<char *>(&buffer[0]), fileSize);
    file.close();

    return buffer;
}
