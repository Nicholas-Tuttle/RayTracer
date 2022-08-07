#include "png.h"
#include "CPURenderer.h"
#include "Image.h"
#include "Ray.h"
#include "IIntersectable.h"
#include "Color.h"
#include "IWorld.h"
#include "IMaterial.h"
#include "Material.h"
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

using RayTracer::CPURenderer;
using RayTracer::Ray;
using RayTracer::Ray;
using RayTracer::Camera;
using RayTracer::IWorld;
using RayTracer::IIntersection;
using RayTracer::IScene;
using RayTracer::IMaterial;
using RayTracer::Material;
using RayTracer::Color;
using RayTracer::IImage;
using RayTracer::Image;

static const IMaterial *GetMaterial(const std::unique_ptr<IScene> &scene, int materialIndex)
{
	const auto& materials = scene->Materials();
	if (materialIndex < 0 || materialIndex >= materials.size())
	{
		return &Material::DefaultMaterial;
	}
	else
	{
		return materials.at(materialIndex);
	}
}

static void TraceRay(const Ray &ray, const std::unique_ptr<IScene> &scene, Color &ray_color)
{
	const unsigned int max_bounces = 10;

	bool at_least_one_intersected = false; 
	bool emissive_material_intersected = false;
	unsigned int total_bounces = 0;
	Ray traced_ray = ray;

	/* Keep going while an intersection happens
	* and the bounces is less than max bounce count */
	do
	{
		bool object_intersected_this_ray = false;
		float min_depth = std::numeric_limits<float>::infinity();
		Ray min_depth_ray{};
		Color min_depth_color{};
		
		// Check for object intersections
		for (const auto &intersectable : scene->Objects())
		{
			std::unique_ptr<IIntersection> intersection = nullptr;
			bool intersected = intersectable->IntersectsRay(&traced_ray, intersection);

			if (intersected)
			{
				if (intersection->Depth() < min_depth)
				{
					min_depth = intersection->Depth();
					min_depth_ray = Ray::ReflectionRay(traced_ray, intersection.get());
					const IMaterial *mat = GetMaterial(scene, intersectable->MaterialIndex());
					min_depth_color = mat->SurfaceColor();
					emissive_material_intersected = mat->Emissive();
				}

				object_intersected_this_ray = true;
			}
		}

		at_least_one_intersected = object_intersected_this_ray;

		if (object_intersected_this_ray)
		{
			traced_ray = min_depth_ray;
			ray_color *= min_depth_color;
		}
		else
		{
			// Intersect with the world
			const IWorld *world = scene->World();
			ray_color *= world->SurfaceColor(traced_ray.Direction());
			break;
		}

		total_bounces++;
		if (total_bounces == max_bounces)
		{
			const IWorld *world = scene->World();
			ray_color *= world->AmbientColor();
			break;
		}
	} 
	while (at_least_one_intersected && !emissive_material_intersected);
}

bool CPURenderer::Render(const Camera &camera, unsigned int samples, const std::unique_ptr<IScene> &scene, std::unique_ptr<IImage> &out_image)
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

	Image *output_image = new Image(camera.Resolution());

	std::vector<Pixel> pixels = camera.GetOutgoingPixels();

	PRINT_TIME("\t[SETUP]: Getting outgoing pixels");

	size_t pixel_number = 1; // Start at 1 so that 0% doesn't print
	size_t ten_percent = pixels.size() / 10;

	// For each sample in each pixel, trace its ray
	for (auto &pixel : pixels)
	{
		if (0 == pixel_number % ten_percent)
		{
			std::string message = "\t[RENDERING]: " + std::to_string(pixel_number / ten_percent * 10) + "% rendered";
			PRINT_TIME(message);
		}

		for (unsigned int i = 0; i < samples; i++)
		{
			Color current_sample_color(1.0f, 1.0f, 1.0f, 1.0f);
			Ray ray = pixel.GetNextRay();
			TraceRay(ray, scene, current_sample_color);
			pixel.AccumulateColorSample(current_sample_color);
		}

		output_image->SetPixelColor(pixel.XCoordinate(), pixel.YCoordinate(), pixel.OutputColor());

		pixel_number++;
	}

	PRINT_TIME("[RENDER TIME]");

	out_image = std::unique_ptr<IImage>(output_image);

	return true;
}