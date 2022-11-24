#pragma once

namespace RayTracer
{
	class IGPUMaterial
	{
	public:
		virtual void CalculateMaterial() const = 0;
	};
}
