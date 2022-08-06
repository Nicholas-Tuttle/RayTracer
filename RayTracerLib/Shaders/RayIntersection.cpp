#include "RayIntersection.h"

using RayTracer::RayIntersectionComputeShader;

const static std::string ShaderFileName = "RayIntersection.comp.spv";

RayIntersectionComputeShader::RayIntersectionComputeShader(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t queueFamilyIndex, 
    VkPipelineLayout &pipelineLayout, VkPipeline &pipeline, VkDescriptorSet &descriptorSet, const uint32_t sampleCount, 
    const InputCamera &camera, const std::vector<InputSphere> &spheres, const std::vector<InputMaterial> &materials, InputResolution resolution)
    : PhysicalDevice(physicalDevice), Device(device)
{
    BufferData.resize((size_t)BufferBindings::BufferBindingsCount);

    BufferData[(int)BufferBindings::input_sample_count].buffer_size = sizeof(InputSampleCount);
    BufferData[(int)BufferBindings::input_sample_count].usage_flag_bits = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    BufferData[(int)BufferBindings::input_sample_count].descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    BufferData[(int)BufferBindings::input_sample_count].data_pointer = (void**)&input_sample_count;

    BufferData[(int)BufferBindings::input_resolution].buffer_size = sizeof(InputResolution);
    BufferData[(int)BufferBindings::input_resolution].usage_flag_bits = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    BufferData[(int)BufferBindings::input_resolution].descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    BufferData[(int)BufferBindings::input_resolution].data_pointer = (void **)&input_resolution;

    BufferData[(int)BufferBindings::input_camera_data].buffer_size = sizeof(InputCamera);
    BufferData[(int)BufferBindings::input_camera_data].usage_flag_bits = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    BufferData[(int)BufferBindings::input_camera_data].descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    BufferData[(int)BufferBindings::input_camera_data].data_pointer = (void **)&input_camera;

    BufferData[(int)BufferBindings::input_sphere_buffer].buffer_size = sizeof(InputSphere) * spheres.size();
    BufferData[(int)BufferBindings::input_sphere_buffer].usage_flag_bits = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    BufferData[(int)BufferBindings::input_sphere_buffer].descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    BufferData[(int)BufferBindings::input_sphere_buffer].data_pointer = (void **)&input_spheres;

    BufferData[(int)BufferBindings::input_material_buffer].buffer_size = sizeof(InputMaterial) * materials.size();
    BufferData[(int)BufferBindings::input_material_buffer].usage_flag_bits = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    BufferData[(int)BufferBindings::input_material_buffer].descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    BufferData[(int)BufferBindings::input_material_buffer].data_pointer = (void **)&input_materials;

    BufferData[(int)BufferBindings::output_color_buffer].buffer_size = sizeof(OutputColor) * resolution.X * resolution.Y;
    BufferData[(int)BufferBindings::output_color_buffer].usage_flag_bits = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    BufferData[(int)BufferBindings::output_color_buffer].descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    BufferData[(int)BufferBindings::output_color_buffer].data_pointer = (void **)&output_colors;

    if (VK_SUCCESS != CreateAndMapMemories(queueFamilyIndex))
    {
        throw std::exception("Failed to create and map memories for compute shader");
    }

    if (nullptr == input_sample_count || nullptr == input_resolution || nullptr == input_camera ||
        nullptr == input_spheres || nullptr == input_materials || nullptr == output_colors)
    {
        throw std::exception("Data pointer for compute shader was nullptr");
    }

    input_resolution->X = resolution.X;
    input_resolution->Y = resolution.Y;
    input_sample_count->sample_count = sampleCount;
    memcpy(input_camera, &camera, sizeof(InputCamera));
    memcpy(input_spheres, spheres.data(), spheres.size() * sizeof(InputSphere));
    memcpy(input_materials, materials.data(), materials.size() * sizeof(InputMaterial));

    if (VK_SUCCESS != CreateShaderModule())
    {
        throw std::exception("Failed to create shader module");
    }

    if (VK_SUCCESS != DescribeShader())
    {
        throw std::exception("Failed to describe shader");
    }

    if (VK_SUCCESS != CreatePipelineLayout(pipelineLayout, pipeline))
    {
        throw std::exception("Failed to create pipeline layout");
    }

    if (VK_SUCCESS != AllocateAndUpdateDescriptorSets(descriptorSet))
    {
        throw std::exception("Failed to allocate and update descriptor sets");
    }
}

VkResult RayIntersectionComputeShader::CreateAndMapMemory(uint32_t queueFamilyIndex,
    const VkDeviceSize memorySize, const VkBufferUsageFlags usage_flags, VkBuffer& vk_buffer, VkDeviceMemory& device_memory, void** out_mapped_memory)
{
    if (memorySize == 0)
    {
        out_mapped_memory = nullptr;
        return VK_SUCCESS;
    }

    VkPhysicalDeviceMemoryProperties properties = {};
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &properties);

    // set memoryTypeIndex to an invalid entry in the properties.memoryTypes array
    uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;

    for (uint32_t k = 0; k < properties.memoryTypeCount; k++)
    {
        if ((VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & properties.memoryTypes[k].propertyFlags) &&
            (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & properties.memoryTypes[k].propertyFlags) &&
            (VK_MEMORY_PROPERTY_HOST_CACHED_BIT & properties.memoryTypes[k].propertyFlags) &&
            (memorySize < properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size))
        {
            memoryTypeIndex = k;
            break;
        }
    }

    if (memoryTypeIndex == VK_MAX_MEMORY_TYPES)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memorySize;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    if (VK_SUCCESS != vkAllocateMemory(Device, &memoryAllocateInfo, 0, &device_memory))
    {
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = memorySize;
    bufferCreateInfo.usage = usage_flags;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = 1;
    bufferCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;

    if (VK_SUCCESS != vkCreateBuffer(Device, &bufferCreateInfo, 0, &vk_buffer))
    {
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    if (VK_SUCCESS != vkBindBufferMemory(Device, vk_buffer, device_memory, 0))
    {
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    return vkMapMemory(Device, device_memory, 0, memorySize, 0, out_mapped_memory);
}

VkResult RayIntersectionComputeShader::CreateAndMapMemories(uint32_t queueFamilyIndex)
{
    VkResult result = VK_SUCCESS;

    for (auto &buffer_data : BufferData)
    {
        if (VK_SUCCESS != CreateAndMapMemory(queueFamilyIndex, buffer_data.buffer_size, buffer_data.usage_flag_bits,
            buffer_data.buffer, buffer_data.device_memory, buffer_data.data_pointer))
        {
            result = VK_ERROR_MEMORY_MAP_FAILED;
        }
    }

    return result;
}

VkResult RayIntersectionComputeShader::CreateShaderModule()
{
    std::vector<uint8_t> shaderCode = ReadShaderFileToBytes(ShaderFileName);
    if (0 == shaderCode.size())
    {
        return VK_ERROR_UNKNOWN;
    }

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shaderCode.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(shaderCode.data());

    return vkCreateShaderModule(Device, &shaderModuleCreateInfo, 0, &ShaderModule);
}

VkResult RayIntersectionComputeShader::DescribeShader()
{
    VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[(size_t)BufferBindings::BufferBindingsCount] = {};

    for (uint32_t i = 0; i < BufferData.size(); i++)
    {
        descriptorSetLayoutBindings[i].binding = i;
        descriptorSetLayoutBindings[i].descriptorType = BufferData[i].descriptor_type;
        descriptorSetLayoutBindings[i].descriptorCount = 1;
        descriptorSetLayoutBindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = (size_t)BufferBindings::BufferBindingsCount;
    descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;

    return vkCreateDescriptorSetLayout(Device, &descriptorSetLayoutCreateInfo, 0, &DescriptorSetLayout);
}

VkResult RayIntersectionComputeShader::CreatePipelineLayout(VkPipelineLayout &pipelineLayout, VkPipeline& pipeline)
{
    VkResult result = VK_SUCCESS;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;

    result = vkCreatePipelineLayout(Device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (VK_SUCCESS != result)
    {
        return result;
    }

    VkComputePipelineCreateInfo computePipelineCreateInfo = {};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computePipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computePipelineCreateInfo.stage.module = ShaderModule;
    computePipelineCreateInfo.stage.pName = "main";
    computePipelineCreateInfo.layout = pipelineLayout;

    return vkCreateComputePipelines(Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, VK_NULL_HANDLE, &pipeline);
}

VkResult RayIntersectionComputeShader::AllocateDescriptorSets(VkDescriptorSet& descriptorSet)
{
    VkDescriptorPoolSize descriptorPoolSizes[(size_t)BufferBindings::BufferBindingsCount] = {};

    for (uint32_t i = 0; i < BufferData.size(); i++)
    {
        descriptorPoolSizes[i].type = BufferData[i].descriptor_type;
        descriptorPoolSizes[i].descriptorCount = 1;
    }

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.poolSizeCount = (uint32_t)BufferBindings::BufferBindingsCount;
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;

    VkDescriptorPool descriptorPool = nullptr;
    VkResult result = VK_SUCCESS;
    result = vkCreateDescriptorPool(Device, &descriptorPoolCreateInfo, 0, &descriptorPool);
    if (VK_SUCCESS != result)
    {
        return result;
    }

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &DescriptorSetLayout;

    return vkAllocateDescriptorSets(Device, &descriptorSetAllocateInfo, &descriptorSet);
}

void RayIntersectionComputeShader::UpdateDescriptorSets(VkDescriptorSet &descriptorSet)
{
    VkDescriptorBufferInfo writeDescriptorSetBufferInfo[(size_t)BufferBindings::BufferBindingsCount] = { 0 };
    VkWriteDescriptorSet writeDescriptorSet[(size_t)BufferBindings::BufferBindingsCount] = {};

    for (uint32_t i = 0; i < BufferData.size(); i++)
    {
        writeDescriptorSetBufferInfo[i].buffer = BufferData[i].buffer;
        writeDescriptorSetBufferInfo[i].offset = 0;
        writeDescriptorSetBufferInfo[i].range = VK_WHOLE_SIZE;
        
        writeDescriptorSet[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[i].dstSet = descriptorSet;
        writeDescriptorSet[i].dstBinding = i;
        writeDescriptorSet[i].descriptorCount = 1;
        writeDescriptorSet[i].descriptorType = BufferData[i].descriptor_type;
        writeDescriptorSet[i].pBufferInfo = &writeDescriptorSetBufferInfo[i];
    }

    vkUpdateDescriptorSets(Device, (uint32_t)BufferBindings::BufferBindingsCount, writeDescriptorSet, 0, nullptr);
}

VkResult RayIntersectionComputeShader::AllocateAndUpdateDescriptorSets(VkDescriptorSet &descriptorSet)
{
    VkResult result = AllocateDescriptorSets(descriptorSet);
    if (VK_SUCCESS != result)
    {
        return result;
    }

    UpdateDescriptorSets(descriptorSet);

    return VK_SUCCESS;
}

const RayIntersectionComputeShader::OutputColor* RayIntersectionComputeShader::GetOutputColors() const
{
    return output_colors;
}

void RayIntersectionComputeShader::UnmapAndDestroyMemories()
{
    for (auto &buffer_data : BufferData)
    {
        if (nullptr != buffer_data.device_memory)
        {
            vkUnmapMemory(Device, buffer_data.device_memory);
            vkFreeMemory(Device, buffer_data.device_memory, nullptr);
        }
    }
}
