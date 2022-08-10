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
using RayTracer::IRay;
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

static void TraceRay(std::unique_ptr<IRay> &ray, const std::unique_ptr<IScene> &scene, Color &ray_color)
{
	const unsigned int max_bounces = 10;

	bool at_least_one_intersected = false;
	unsigned int total_bounces = 0;
	std::unique_ptr<IRay> &traced_ray = ray;

	/* Keep going while an intersection happens
	* and the bounces is less than max bounce count */
	while (nullptr != traced_ray)
	{
		at_least_one_intersected = false;
		float min_depth = std::numeric_limits<float>::infinity();
		Color min_depth_color{};
		std::unique_ptr<IRay> reflected_ray = nullptr;
		
		// Check for object intersections
		for (const auto &intersectable : scene->Objects())
		{
			std::unique_ptr<IIntersection> intersection = nullptr;
			bool intersected = intersectable->IntersectsRay(traced_ray, intersection);

			if (intersected)
			{
				if (intersection->Depth() < min_depth)
				{
					min_depth = intersection->Depth();
					const IMaterial *mat = GetMaterial(scene, intersectable->MaterialIndex());
					min_depth_color = mat->SurfaceColor();
					mat->GetResultantRay(intersection, traced_ray, reflected_ray);
				}

				at_least_one_intersected = true;
			}
		}

		// TODO: the surfaces should modulate with the color, not just multiply (add specular reflections)

		if (at_least_one_intersected)
		{
			traced_ray.swap(reflected_ray);
			ray_color *= min_depth_color;
		}
		else
		{
			// Intersect with the world
			const IWorld *world = scene->World();
			ray_color *= world->SurfaceColor(traced_ray->Direction());
			traced_ray = nullptr;
		}

		total_bounces++;
		if (total_bounces == max_bounces)
		{
			const IWorld *world = scene->World();
			ray_color *= world->AmbientColor();
			break;
		}
	}
}

void CPURenderer::Render(const Camera &camera, unsigned int samples, const std::unique_ptr<IScene> &scene, std::unique_ptr<IImage> &out_image)
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

	out_image = std::make_unique<Image>(camera.Resolution());

	std::vector<Pixel> pixels = camera.GetOutgoingPixels();

	PRINT_TIME("\t[SETUP]: Getting outgoing pixels");

	std::cout << "[RENDERING]" << std::endl;

	// For each sample in each pixel, trace its ray
	// TODO: Threading
	for (auto &pixel : pixels)
	{
		for (unsigned int i = 0; i < samples; i++)
		{
			Color current_sample_color(1.0f, 1.0f, 1.0f, 1.0f);
			std::unique_ptr<IRay> ray = pixel.GetNextRay();
			TraceRay(ray, scene, current_sample_color);
			pixel.AccumulateColorSample(current_sample_color);
		}

		out_image->SetPixelColor(pixel.XCoordinate(), pixel.YCoordinate(), pixel.OutputColor());
	}

	PRINT_TIME("[RENDER TIME]");
}