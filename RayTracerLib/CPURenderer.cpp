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
using RayTracer::Camera;
using RayTracer::IWorld;
using RayTracer::Intersection;
using RayTracer::IScene;
using RayTracer::IMaterial;
using RayTracer::Material;
using RayTracer::Color;
using RayTracer::IImage;
using RayTracer::Image;
using RayTracer::PixelRenderTask;

void CPURenderer::Render(size_t max_threads, const Camera &camera, unsigned int samples, 
	const std::shared_ptr<IScene> scene, std::shared_ptr<IImage> &out_image)
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
	if (max_threads > 0)
	{
		hardware_concurrency = std::min<unsigned int>((unsigned int)max_threads, hardware_concurrency);
	}
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