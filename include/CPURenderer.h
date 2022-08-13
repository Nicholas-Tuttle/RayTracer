#pragma once

#include "IImage.h"
#include "IScene.h"
#include "Camera.h"

namespace RayTracer
{
	class CPURenderer
	{
	public:
		void Render(const Camera& camera, unsigned int samples, const std::shared_ptr<IScene> scene, std::shared_ptr<IImage> &out_image);
	};
}

