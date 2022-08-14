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
using RayTracer::Vector3;

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

static void TraceRay(std::unique_ptr<IRay> &ray, const std::shared_ptr<IScene> scene)
{
	const unsigned int max_bounces = 10;

	unsigned int total_bounces = 0;
	std::unique_ptr<IRay> &traced_ray = ray;

	/* Keep going while an intersection happens
	* and the bounces is less than max bounce count */
	while (nullptr != traced_ray && traced_ray->Direction() != Vector3<float>(0, 0, 0))
	{
		float min_depth = std::numeric_limits<float>::infinity();

		const IMaterial *closest_intersection_mat = nullptr;
		std::unique_ptr<IIntersection> closest_intersection = nullptr;

		// Check for object intersections
		for (const auto &intersectable : scene->Objects())
		{
			std::unique_ptr<IIntersection> current_intersection = nullptr;
			if (intersectable->IntersectsRay(traced_ray, current_intersection)
				&& current_intersection->Depth() < min_depth)
			{
				closest_intersection.swap(current_intersection);
				min_depth = closest_intersection->Depth();
				closest_intersection_mat = GetMaterial(scene, intersectable->MaterialIndex());
			}
		}

		if (nullptr != closest_intersection_mat)
		{
			std::unique_ptr<IRay> reflected_ray = nullptr;
			closest_intersection_mat->GetResultantRay(closest_intersection, traced_ray, reflected_ray);
			traced_ray.swap(reflected_ray);
		}
		else
		{
			// Intersect with the world
			const IWorld *world = scene->World();
			traced_ray->RayColor() *= world->SurfaceColor(traced_ray->Direction());
			break;
		}

		total_bounces++;
		if (total_bounces == max_bounces)
		{
			const IWorld *world = scene->World();
			traced_ray->RayColor() *= world->AmbientColor();
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
		TraceRay(ray, scene);
		colors[i] = ray->RayColor();
	}
	
	pixel.Average(colors);

	out_image->SetPixelColor(pixel.XCoordinate(), pixel.YCoordinate(), pixel.OutputColor());
}