#pragma once

#include "IGPUMaterial.h"

namespace RayTracer
{
	class GPUDiffuseMaterial : public IGPUMaterial
	{
		virtual void CalculateMaterial() const 
		{
			std::cout << __FUNCTION__ << std::endl;
		}
	};
}

