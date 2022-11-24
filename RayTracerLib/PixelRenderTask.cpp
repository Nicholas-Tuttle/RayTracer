#include "PixelRenderTask.h"

using RayTracer::ThreadPool;
using RayTracer::PixelRenderTask;
using RayTracer::Pixel;
using RayTracer::Ray;
using RayTracer::IScene;
using RayTracer::Color;
using RayTracer::Intersection;
using RayTracer::IMaterial;
using RayTracer::IWorld;
using RayTracer::IImage;
using RayTracer::Material;
using RayTracer::Vector3;

PixelRenderTask::PixelRenderTask(Pixel &pixel, unsigned int samples, const IScene &scene, std::shared_ptr<IImage> out_image) :
	pixel(pixel), samples(samples), scene(scene), out_image(out_image) {}

static const IMaterial *GetMaterial(const IScene &scene, int materialIndex)
{
	const auto &materials = scene.Materials();
	if (materialIndex < 0 || materialIndex >= materials.size())
	{
		return &Material::DefaultMaterial;
	}
	else
	{
		return materials.at(materialIndex);
	}
}

static void TraceRay(Ray &ray, const IScene &scene)
{
	const unsigned int max_bounces = 10;

	unsigned int total_bounces = 0;
	Ray &traced_ray = ray;

	/* Keep going while an intersection happens
	* and the bounces is less than max bounce count */
	while (traced_ray.Direction() != Vector3<float>(0, 0, 0))
	{
		float min_depth = std::numeric_limits<float>::infinity();

		const IMaterial *closest_intersection_mat = nullptr;
		Intersection closest_intersection;

		// Check for object intersections
		for (const auto &intersectable : scene.Objects())
		{
			Intersection current_intersection;
			if (intersectable->IntersectsRay(traced_ray, current_intersection)
				&& current_intersection.Depth() < min_depth)
			{
				closest_intersection = current_intersection;
				min_depth = closest_intersection.Depth();
				closest_intersection_mat = GetMaterial(scene, intersectable->MaterialIndex());
			}
		}

		if (nullptr != closest_intersection_mat)
		{
			Ray reflected_ray;
			closest_intersection_mat->GetResultantRay(closest_intersection, traced_ray, reflected_ray);
			traced_ray = reflected_ray;
		}
		else
		{
			// Intersect with the world
			const IWorld *world = scene.World();
			traced_ray.SetColor(traced_ray.RayColor() * world->SurfaceColor(traced_ray.Direction()));
			break;
		}

		total_bounces++;
		if (total_bounces == max_bounces)
		{
			const IWorld *world = scene.World();
			traced_ray.SetColor(traced_ray.RayColor() * world->AmbientColor());
			break;
		}
	}
}

void PixelRenderTask::Execute()
{
	std::vector<Color> colors;
	colors.reserve(samples);
	for (unsigned int i = 0; i < samples; i++)
	{
		Ray ray = pixel.GetNextRay();
		TraceRay(ray, scene);
		colors.emplace_back(ray.RayColor());
	}
	
	pixel.Average(colors);

	out_image->SetPixelColor(pixel.XCoordinate(), pixel.YCoordinate(), pixel.OutputColor());
}