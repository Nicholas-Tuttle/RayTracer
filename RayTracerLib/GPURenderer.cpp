#include "GPURenderer.h"
#include <chrono>
#include <iostream>
#include <Sphere.h>

#include "Shaders/RayIntersection.h"

using RayTracer::GPURenderer;
using RayTracer::Camera;
using RayTracer::IScene;
using RayTracer::Sphere;
using RayTracer::Image;
using RayTracer::Color;
using RayTracer::RayIntersectionComputeShader;

#define EXIT_ON_BAD_RESULT(result) if (VK_SUCCESS != result) { fprintf(stderr, "Failure at %u %s\n", __LINE__, __FILE__); exit(-1); }
#define RETURN_ON_BAD_RETCODE(retcode) do { if(VK_SUCCESS != retcode) { return retcode; } }while(0)

#define SHOW_ONLY_DEBUG_PRINTF_EXT_MESSAGES false

static const size_t shader_local_size_x = 1024;

// This Vulkan debug callback receives messages from the 
// debugPrintfEXT (GLSL) or printf (HLSL) functions in the
// compute shaders, along with other vulkan messages. 
// For more reference, see:
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerEXT.html
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCreateDebugUtilsMessengerEXT.html
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkDestroyDebugUtilsMessengerEXT.html
// for more info
static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData,
    void *pUserData
)
{
#if SHOW_ONLY_DEBUG_PRINTF_EXT_MESSAGES
    // NOTE:
    // This message filtering can (and probably should) be done as part of the 
    // intialization in "vkCreateDebugUtilsMessengerEXT", using the 
    // VkDebugUtilsMessengerCreateInfoEXT.messageType variable. 
    // The initialization in "CreateDebugMessenger()" does not do any filtering 
    // and it is instead done here to demonstrate one potential usage of the 
    // messageType parameter, but is not optimal.
    if (VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT != messageType)
    {
        return VK_FALSE;
    }
#endif

    std::cout << "[VULKAN DEBUG] : ";

    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    {
        std::cout << "[VERBOSE]";
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    {
        std::cout << "[INFO]   ";
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    {
        std::cout << "[WARNING]";
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    {
        std::cout << "[ERROR]  ";
        break;
    }
    default:
    {
        std::cout << "[UNKNOWN]";
        break;
    }
    }

    std::cout << " : [FLAGS]: " << messageType << "\t" << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

// Verifies the instance layers in requiredInstanceLayers are available
bool GPURenderer::VerifyInstanceLayers()
{
    if (GPUDebugEnabled)
    {
        // VK_LAYER_KHRONOS_validation device extension must be enabled
        requiredInstanceLayers.emplace_back("VK_LAYER_KHRONOS_validation");
    }
    
    if (requiredInstanceLayers.size() == 0)
    {
        return true;
    }

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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

// Verifies the instance extensions in requiredInstanceExtensions are available
bool GPURenderer::VerifyInstanceExtensions()
{
    if (GPUDebugEnabled)
    {
        // VK_EXT_debug_utils instance extensions must be enabled for the VulkanDebugCallback function
        requiredInstanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (requiredInstanceExtensions.size() == 0)
    {
        return true;
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

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

// Creates a Vulkan Debug Messenger that receives all messages
VkResult GPURenderer::CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT *debugMessenger)
{
    if (nullptr == instance || nullptr == debugMessenger)
    {
        return VK_ERROR_UNKNOWN;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    createInfo.pfnUserCallback = VulkanDebugCallback;
    createInfo.pUserData = nullptr;

    // The function must by loaded dynamically by name
    auto function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (function != nullptr)
    {
        // call dll function vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, debugMessenger);
        return function(instance, &createInfo, nullptr, debugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// Destroys a previously created Vulkan Debug Messenger
void GPURenderer::DestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger)
{
    if (nullptr == instance || nullptr == debugMessenger)
    {
        return;
    }

    auto function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    // call dll function vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    function(instance, debugMessenger, nullptr);
}

VkResult GPURenderer::CreateHeadlessVulkanInstance(VkInstance & instance)
{
    VkResult result = VK_SUCCESS;

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = 0;
    applicationInfo.pApplicationName = "GPURenderer";
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = "";
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_HEADER_VERSION_COMPLETE;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;

#if ENABLE_SHADER_PRINTF
    createInfo.enabledLayerCount = static_cast<uint32_t>(requiredInstanceLayers.size());
    createInfo.ppEnabledLayerNames = requiredInstanceLayers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();

    VkValidationFeatureEnableEXT enabled = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.disabledValidationFeatureCount = 0;
    features.enabledValidationFeatureCount = 1;
    features.pDisabledValidationFeatures = nullptr;
    features.pEnabledValidationFeatures = &enabled;

    createInfo.pNext = &features;
#endif

    result =  vkCreateInstance(&createInfo, nullptr, &instance);
    return result;
}

VkResult GPURenderer::EnumerateDevices(VkInstance instance, VkPhysicalDevice*& devices, uint32_t& device_count)
{
    VkResult result = VK_SUCCESS;

    result = vkEnumeratePhysicalDevices(instance, &device_count, 0);
    RETURN_ON_BAD_RETCODE(result);

    devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * device_count);

    return vkEnumeratePhysicalDevices(instance, &device_count, devices);
}

VkResult GPURenderer::GetBestComputeQueue(VkPhysicalDevice physicalDevice, uint32_t& queueFamilyIndex)
{
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, 0);

    VkQueueFamilyProperties* const queueFamilyProperties = (VkQueueFamilyProperties*)_malloca(sizeof(VkQueueFamilyProperties) * queueFamilyPropertiesCount);
    if (nullptr == queueFamilyProperties)
    {
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties);

    // first try and find a queue that has just the compute bit set
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++)
    {
        // mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
        const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) & queueFamilyProperties[i].queueFlags);

        if (!(VK_QUEUE_GRAPHICS_BIT & maskedFlags) && (VK_QUEUE_COMPUTE_BIT & maskedFlags))
        {
            queueFamilyIndex = i;
            return VK_SUCCESS;
        }
    }

    // lastly get any queue that'll work for us
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++)
    {
        // mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
        const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) & queueFamilyProperties[i].queueFlags);

        if (VK_QUEUE_COMPUTE_BIT & maskedFlags)
        {
            queueFamilyIndex = i;
            return VK_SUCCESS;
        }
    }

    return VK_ERROR_INITIALIZATION_FAILED;
}

VkResult GPURenderer::CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkDevice& device)
{
    const float queuePrioritory = 1.0f;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &queuePrioritory;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.ppEnabledExtensionNames = nullptr;

    return vkCreateDevice(physicalDevice, &deviceCreateInfo, 0, &device);
}

bool GPURenderer::Render(const Camera& camera, unsigned int samples, const IScene* scene, IImage*& out_image)
{
    auto time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> cpuTime;
#define PRINT_TIME(message) \
do \
{   \
    cpuTime = std::chrono::high_resolution_clock::now() - time; \
    time = std::chrono::high_resolution_clock::now(); \
    std::cout << message << ": line " << __LINE__ << ": time (ms): " << cpuTime.count() << std::endl ;    \
} while(0)

    if (!VerifyInstanceLayers())
    {
        return false;
    }

    if (!VerifyInstanceExtensions())
    {
        return false;
    }

    VkInstance instance = nullptr;
    EXIT_ON_BAD_RESULT(CreateHeadlessVulkanInstance(instance));

#if ENABLE_SHADER_PRINTF
    VkDebugUtilsMessengerEXT debugMessenger = {};
    EXIT_ON_BAD_RESULT(CreateDebugMessenger(instance, &debugMessenger));
#endif

    PRINT_TIME("\t[SETUP]: Create headless vulkan instance");

    VkPhysicalDevice* physicalDevices = nullptr;
    uint32_t physicalDeviceCount = 0;
    EXIT_ON_BAD_RESULT(EnumerateDevices(instance, physicalDevices, physicalDeviceCount));

    if (0 == physicalDeviceCount)
    {
        return false;
    }

    PRINT_TIME("\t[SETUP]: Enumerate devices");

    uint32_t queueFamilyIndex = 0;
    EXIT_ON_BAD_RESULT(GetBestComputeQueue(physicalDevices[0], queueFamilyIndex));

    PRINT_TIME("\t[SETUP]: Get best compute queue");

    VkDevice device = nullptr;
    EXIT_ON_BAD_RESULT(CreateDevice(physicalDevices[0], queueFamilyIndex, device));

    PRINT_TIME("\t[SETUP]: Create device");

    std::vector<RayIntersectionComputeShader::InputSphere> input_spheres;
    for (const auto &object : scene->Objects())
    {
        const Sphere *sphere = dynamic_cast<const Sphere *>(object);
        if (nullptr == sphere)
        {
            continue;
        }

        input_spheres.emplace_back(*sphere);
    }

    std::vector<RayIntersectionComputeShader::InputMaterial> input_materials;
    for (const auto &material : scene->Materials())
    {
        input_materials.emplace_back(*material);
    }

    RayIntersectionComputeShader::InputCamera input_camera(camera);

    VkPipelineLayout pipelineLayout = nullptr;
    VkPipeline pipeline = nullptr; 
    VkDescriptorSet descriptorSet = {};
    RayIntersectionComputeShader ray_intersection_shader(physicalDevices[0], device, queueFamilyIndex, pipelineLayout, pipeline, descriptorSet,
        samples, input_camera, input_spheres, input_materials, {(unsigned int)camera.Resolution().X, (unsigned int)camera.Resolution().Y});
    
    PRINT_TIME("\t[SETUP]: Creating ray intersection shader");

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool;
    EXIT_ON_BAD_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, 0, &commandPool));

    PRINT_TIME("\t[SETUP]: Creating command pool");

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = {};
    EXIT_ON_BAD_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

    PRINT_TIME("\t[SETUP]: Allocating command buffers");

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    EXIT_ON_BAD_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, 0);

    uint32_t group_count_x = (uint32_t)std::ceil((float)(camera.Resolution().X * camera.Resolution().Y) / (float)shader_local_size_x);
    vkCmdDispatch(commandBuffer, group_count_x, 1, 1);
    EXIT_ON_BAD_RESULT(vkEndCommandBuffer(commandBuffer));

    PRINT_TIME("\t[SETUP]: Creating command buffer");

    VkQueue queue = {};
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    auto tm0 = std::chrono::high_resolution_clock::now();

    EXIT_ON_BAD_RESULT(vkQueueSubmit(queue, 1, &submitInfo, 0));
    EXIT_ON_BAD_RESULT(vkQueueWaitIdle(queue));

    PRINT_TIME("[RENDER TIME]");

    Image* output_image = new Image(camera.Resolution());
    const RayIntersectionComputeShader::OutputColor* output_colors = ray_intersection_shader.GetOutputColors();
    for (int y = 0; y < camera.Resolution().Y; y++)
    {
        for (int x = 0; x < camera.Resolution().X; x++)
        {
            size_t index = y * camera.Resolution().X + x;

            float r = output_colors[index].rgb[0];
            float g = output_colors[index].rgb[1];
            float b = output_colors[index].rgb[2];

            output_image->SetPixelColor(x, y, Color(r, g, b, 1.0));
        }
    }
    out_image = output_image;

    PRINT_TIME("\t[TEARDOWN]: Reading results");

    ray_intersection_shader.UnmapAndDestroyMemories();
    
    PRINT_TIME("\t[TEARDOWN]: Unmap memory");

#if ENABLE_SHADER_PRINTF
    DestroyDebugMessenger(instance, debugMessenger);
#endif

	return true;
}

#undef EXIT_ON_BAD_RESULT