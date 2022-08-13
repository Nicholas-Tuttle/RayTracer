#include "png.h"
#include "CPURenderer.h"
#include "Image.h"
#include "Ray.h"
#include "IIntersectable.h"
#include "Color.h"
#include "IWorld.h"
#include "IMaterial.h"
#include "Material.h"
#include "ThreadPool.h"
#include "PixelRenderTask.h"
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <thread>

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
using RayTracer::PixelRenderTask;

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

void CPURenderer::Render(const Camera &camera, unsigned int samples, const std::shared_ptr<IScene> scene, std::shared_ptr<IImage> &out_image)
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

	out_image = std::make_shared<Image>(camera.Resolution());

	std::vector<Pixel> pixels = camera.GetOutgoingPixels();

	PRINT_TIME("\t[SETUP]: Getting outgoing pixels");

	std::cout << "[RENDERING]" << std::endl;

	// For each sample in each pixel, trace its ray
	unsigned int hardware_concurrency = std::thread::hardware_concurrency();
	// If there is more than one core, leave one available for enquing other tasks
	hardware_concurrency -= hardware_concurrency > 1 ? 1 : 0;

	ThreadPool rendering_pool(hardware_concurrency, 1000);

	for (auto &pixel : pixels)
	{
		std::shared_ptr<ThreadPool::IThreadPoolTask> pixel_render_task = std::make_shared<PixelRenderTask>(pixel, samples, scene, out_image);
		rendering_pool.EnqueueTask(pixel_render_task);
	}

	rendering_pool.BlockUntilComplete();

	PRINT_TIME("[RENDER TIME]");
}