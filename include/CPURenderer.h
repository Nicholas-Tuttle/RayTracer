#pragma once

#include "IImage.h"
#include "IScene.h"
#include "Camera.h"

namespace RayTracer
{
	class CPURenderer
	{
	public:
		bool Render(const Camera& camera, unsigned int samples, const IScene* scene, IImage*& out_image);
	};
}

