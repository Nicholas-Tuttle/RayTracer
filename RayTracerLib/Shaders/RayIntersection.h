#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <fstream>
#include <Sphere.h>
#include <IMaterial.h>
#include <Utilities.h>
#include <Camera.h>

namespace RayTracer
{
	class RayIntersectionComputeShader
	{
	public:
		struct InputSampleCount
		{
			uint32_t sample_count;
		};

		struct InputResolution
		{
			unsigned int X;
			unsigned int Y;
		};

		struct InputCamera
		{
			InputCamera(const Camera &camera)
			{
				focal_length_mm = 50;
				sensor_width_mm = 18;
				origin_x = camera.Position().X;
				origin_y = camera.Position().Y;
				origin_z = camera.Position().Z;
				forward_vector_x = camera.ForwardVector().X;
				forward_vector_y = camera.ForwardVector().Y;
				forward_vector_z = camera.ForwardVector().Z;
				right_vector_x = camera.RightVector().X;
				right_vector_y = camera.RightVector().Y;
				right_vector_z = camera.RightVector().Z;
				up_vector_x = camera.UpVector().X;
				up_vector_y = camera.UpVector().Y;
				up_vector_z = camera.UpVector().Z;
			}

			unsigned int focal_length_mm;
			unsigned int sensor_width_mm;
			float origin_x;
			float origin_y;
			float origin_z;
			float forward_vector_x;
			float forward_vector_y;
			float forward_vector_z;
			float right_vector_x;
			float right_vector_y;
			float right_vector_z;
			float up_vector_x;
			float up_vector_y;
			float up_vector_z;
		};

		struct InputSphere
		{
			InputSphere(const Sphere &sphere)
			{
				Vector3<float> sphere_position = sphere.Position();
				position[0] = sphere_position.X;
				position[1] = sphere_position.Y;
				position[2] = sphere_position.Z;

				radius = sphere.Radius();
				material_index = sphere.MaterialIndex();
			}

			float position[3];
			float radius;
			int material_index;
		};

		struct InputMaterial
		{
			InputMaterial(const IMaterial &material)
			{
				color[0] = material.SurfaceColor().R_float();
				color[1] = material.SurfaceColor().G_float();
				color[2] = material.SurfaceColor().B_float();
				roughness = material.Roughness();
			}

			float color[3];
			float roughness;
		};

		struct OutputColor
		{
			float rgb[3];
		};

		RayIntersectionComputeShader(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t queueFamilyIndex,
			VkPipelineLayout &pipelineLayout, VkPipeline &pipeline, VkDescriptorSet &descriptorSet,
			const uint32_t sampleCount, const InputCamera &camera, const std::vector<InputSphere> &spheres, 
			const std::vector<InputMaterial> &materials, InputResolution resolution);

		void UnmapAndDestroyMemories();
		const OutputColor* GetOutputColors() const;

	private:
		VkPhysicalDevice PhysicalDevice = nullptr;
		VkDevice Device = nullptr;
		VkShaderModule ShaderModule = nullptr;
		VkDescriptorSetLayout DescriptorSetLayout = nullptr;

		enum class BufferBindings
		{
			input_sample_count,
			input_resolution,
			input_camera_data,
			input_sphere_buffer,
			input_material_buffer,
			output_color_buffer,
			BufferBindingsCount
		};

		struct BufferCreationAndMappingData
		{
			VkDeviceSize buffer_size;
			VkBuffer buffer;
			VkDeviceMemory device_memory;
			VkBufferUsageFlagBits usage_flag_bits;
			VkDescriptorType descriptor_type;
			void **data_pointer;
		};

		std::vector<BufferCreationAndMappingData> BufferData;

		// Input Sample Buffer Data
		InputSampleCount* input_sample_count = nullptr;

		// Input Resolution Buffer Data
		InputResolution *input_resolution = nullptr;

		// Input Camera Buffer Data
		InputCamera *input_camera = nullptr;

		// Input Sphere Buffer Data
		InputSphere* input_spheres = nullptr;

		// Input Material Buffer Data
		InputMaterial *input_materials = nullptr;

		// Output Color Buffer Data
		OutputColor* output_colors = nullptr;

		VkResult CreateAndMapMemory(uint32_t queueFamilyIndex, const VkDeviceSize memorySize, const VkBufferUsageFlags usage_flags,
			VkBuffer& vk_buffer, VkDeviceMemory& device_memory, void** out_mapped_memory);
		VkResult CreateAndMapMemories(uint32_t queueFamilyIndex);
		VkResult CreateShaderModule();
		VkResult DescribeShader();
		VkResult AllocateDescriptorSets(VkDescriptorSet &descriptorSet);
		void UpdateDescriptorSets(VkDescriptorSet &descriptorSet);
		VkResult CreatePipelineLayout(VkPipelineLayout &pipelineLayout, VkPipeline &pipeline);
		VkResult AllocateAndUpdateDescriptorSets(VkDescriptorSet &descriptorSet);
	};
}