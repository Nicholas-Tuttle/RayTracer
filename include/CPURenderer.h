#pragma once

#include "IImage.h"
#include "IScene.h"
#include "Camera.h"

namespace RayTracer
{
	class CPURenderer
	{
	public:
		void Render(size_t max_threads, const Camera& camera, unsigned int samples, 
			const IScene &scene, std::shared_ptr<IImage> &out_image);
	};
}

