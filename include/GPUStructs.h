#pragma once

#include "Sphere.h"
#include "DiffuseBSDF.h"
#include "EmissiveBSDF.h"

#define GPU_PADDING_BYTES(byte_count) char _[byte_count] = { 0 };

namespace RayTracer
{
	// When editing the structs in the shader code calculate the size 
	// of the GPU struct and include alignment in these structs 

#pragma region Ray Tracing Structures

	struct GPURay
	{
		float origin[4];
		float direction[4];
		uint32_t pixelXIndex;
		uint32_t pixelYIndex;
		GPU_PADDING_BYTES(8);
	};

	struct GPUIntersection
	{
		float ray_origin[4];
		float ray_direction[4];
		float intersection_location[4];
		float intersection_normal[4];
		float ray_color[4];
		float depth;
		uint32_t object_id;
		uint32_t face_id;
		uint32_t material_id;
		int32_t material_index;
		GPU_PADDING_BYTES(12);
	};

	struct GPUSphere
	{
		GPUSphere()
		{
			position[0] = 0;
			position[1] = 0;
			position[2] = 0;
			position[3] = 0;
			radius = 0;
			material_id = 0;
		}

		GPUSphere(const Sphere &sphere)
		{
			const Vector3<float> &sphere_position = sphere.Position();
			position[0] = sphere_position.X;
			position[1] = sphere_position.Y;
			position[2] = sphere_position.Z;
			position[3] = 0;

			radius = sphere.Radius();
		}

		GPUSphere(const Sphere *sphere)
		{
			if (nullptr == sphere)
			{
				return;
			}

			const Vector3<float> &sphere_position = sphere->Position();
			position[0] = sphere_position.X;
			position[1] = sphere_position.Y;
			position[2] = sphere_position.Z;
			position[3] = 0;

			radius = sphere->Radius();
		}

		float position[4];
		float radius;
		uint32_t material_id;
		int32_t material_index;
		GPU_PADDING_BYTES(4);
	};

	struct GPUSample
	{
		float color[4];
	};

#pragma endregion

#pragma region Materials

	struct GPUDiffuseMaterialParameters
	{
		float color[4];
		float roughness;
		GPU_PADDING_BYTES(12);

		GPUDiffuseMaterialParameters()
		{
			color[0] = 1;
			color[1] = 1;
			color[2] = 1;
			color[3] = 1;
			roughness = 0;
		}

		GPUDiffuseMaterialParameters(const RayTracer::DiffuseBSDF *diffuse)
			: GPUDiffuseMaterialParameters()
		{
			if (nullptr == diffuse)
			{
				return;
			}

			color[0] = diffuse->SurfaceColor().R_float();
			color[1] = diffuse->SurfaceColor().G_float();
			color[2] = diffuse->SurfaceColor().B_float();
			color[3] = 1;
			roughness = diffuse->Roughness();
		}
	};

	struct GPUEmissiveMaterialParameters
	{
		float color[4];
		float strength;
		GPU_PADDING_BYTES(12);

		GPUEmissiveMaterialParameters()
		{
			color[0] = 1;
			color[1] = 1;
			color[2] = 1;
			color[3] = 1;
			strength = 1;
		}

		GPUEmissiveMaterialParameters(const RayTracer::EmissiveBSDF *emissive)
			: GPUEmissiveMaterialParameters()
		{
			if (nullptr == emissive)
			{
				return;
			}

			color[0] = emissive->SurfaceColor().R_float();
			color[1] = emissive->SurfaceColor().G_float();
			color[2] = emissive->SurfaceColor().B_float();
			color[3] = 1;
			strength = emissive->Strength();
		}
	};

	// Based on Blender's Principled BSDF object material shader
	// TODO: If this doesn't copy to the GPU code correctly the float[4] arrays might need to be ordered differently in the struct
	// TODO: This should replace basically every other surface-shader and they should be removed
	// TODO: Add default values where needed
	struct GPUPrincipledMaterialParameters
	{
		// Basic parameters
		float color[4];
		float roughness;
		float metallic;
		// Specular
		float specular;
		float specular_tint;
		// Anisotropic
		float anisotropic;
		float anisotropic_rotation;
		// Sheen
		float sheen;
		float sheen_tint;
		// Clearcoat
		float clearcoat;
		float clearcoat_roughness;
		// Transmission/transparency
		float index_of_refraction;
		float transmission;
		float transmission_roughness;
		// Emission
		float emission_color[4];
		float emission_strength;
		// Subsurface
		float subsurface_color[4];
		float subsurface_radius[4];
		float subsurface_index_of_refraction;
		float subsurface_anisotropy;
		float subsurface_strength;
		// Geometric
		float normal[4];
		float clearcoat_normal[4];
		float tangent[4];
		// Struct alignment
		GPU_PADDING_BYTES(12);
	};

#pragma endregion
}

#undef GPU_PADDING_BYTES
