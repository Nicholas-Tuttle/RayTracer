#pragma once

#include "IImage.h"
#include "IScene.h"
#include "Camera.h"

namespace RayTracer
{
	class CPURenderer
	{
	public:
		bool Render(const Camera& camera, unsigned int samples, const std::unique_ptr<IScene> &scene, std::unique_ptr<IImage> &out_image);
	};
}

