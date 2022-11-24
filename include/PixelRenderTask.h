#pragma once

#include "ThreadPool.h"
#include "Pixel.h"
#include "Scene.h"
#include "Ray.h"
#include "Material.h"
#include "IImage.h"

namespace RayTracer
{
	class PixelRenderTask : public ThreadPool::IThreadPoolTask
	{
	public:
		PixelRenderTask(Pixel &pixel, unsigned int samples, 
			const IScene &scene, std::shared_ptr<IImage> out_image);
		void Execute() override;
	private:
		Pixel &pixel;
		unsigned int samples;
		const IScene &scene;
		std::shared_ptr<IImage> out_image;
	};
}