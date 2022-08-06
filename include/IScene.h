#pragma once

#include "IIntersectable.h"
#include "IMaterial.h"
#include "IWorld.h"
#include <vector>

namespace RayTracer
{
	class IScene
	{
	public:
		virtual const std::vector<IIntersectable*> &Objects() const = 0;
		virtual const std::vector<IMaterial *> &Materials() const = 0;
		virtual const IWorld *World() const = 0;
	};
}