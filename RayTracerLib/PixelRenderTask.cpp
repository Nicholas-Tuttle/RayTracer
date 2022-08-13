#include "PixelRenderTask.h"

using RayTracer::ThreadPool;
using RayTracer::PixelRenderTask;
using RayTracer::Pixel;
using RayTracer::IRay;
using RayTracer::IScene;
using RayTracer::Color;
using RayTracer::IIntersection;
using RayTracer::IMaterial;
using RayTracer::IWorld;
using RayTracer::IImage;
using RayTracer::Material;

PixelRenderTask::PixelRenderTask(Pixel &pixel, unsigned int samples, std::shared_ptr<IScene> scene, std::shared_ptr<IImage> out_image) :
	pixel(pixel), samples(samples), scene(scene), out_image(out_image) {}

static const IMaterial *GetMaterial(const std::shared_ptr<IScene> scene, int materialIndex)
{
	const auto &materials = scene->Materials();
	if (materialIndex < 0 || materialIndex >= materials.size())
	{
		return &Material::DefaultMaterial;
	}
	else
	{
		return materials.at(materialIndex);
	}
}

static void TraceRay(std::unique_ptr<IRay> &ray, const std::shared_ptr<IScene> scene, Color &ray_color)
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

void PixelRenderTask::Execute()
{
	std::vector<Color> colors;
	colors.resize(samples, Color(1.0f, 1.0f, 1.0f, 1.0f));
	for (unsigned int i = 0; i < samples; i++)
	{
		std::unique_ptr<IRay> ray = pixel.GetNextRay();
		TraceRay(ray, scene, colors.at(i));
	}
	
	pixel.Average(colors);

	out_image->SetPixelColor(pixel.XCoordinate(), pixel.YCoordinate(), pixel.OutputColor());
}