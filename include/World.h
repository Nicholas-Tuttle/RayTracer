#pragma once

#include "IWorld.h"

namespace RayTracer
{
	class World : public IWorld
	{
	public:
		virtual Color SurfaceColor(Vector3<float> normal) const
		{
			return (normal.Y > 0) ? Color(0.7f, 0.9f, 1.0f, 1.0f) : Color(0.6f, 0.3f, 0.15f, 1.0);
		}

		virtual Color AmbientColor() const
		{
			return Color(0.1f, 0.1f, 0.1f, 1.0f);
		}
	};
}
