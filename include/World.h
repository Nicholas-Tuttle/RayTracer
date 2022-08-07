#pragma once

#include "IWorld.h"

namespace RayTracer
{
	class World : public IWorld
	{
	public:
		virtual Color SurfaceColor(const Vector3<float> &normal) const override
		{
			return (normal.Y > 0) ? Color(0.7f, 0.9f, 1.0f, 1.0f) : Color(0.6f, 0.3f, 0.15f, 1.0);
		}

		virtual const Color &AmbientColor() const
		{
			return ambientColor;
		}
	private:
		static Color ambientColor;
	};
}
