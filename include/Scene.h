#pragma once

#include "IScene.h"
#include "World.h"

namespace RayTracer
{
	class Scene : public IScene
	{
	private:
		std::vector<const IIntersectable*> objects;
		const RayTracer::World world;
	public:
		virtual const std::vector<const IIntersectable*> &Objects() const override
		{
			return objects;
		}

		virtual const IWorld *World() const
		{
			return &world;
		}

		void AddObject(const IIntersectable* object)
		{
			objects.emplace_back(object);
		}
	};
}