#pragma once

#include "IScene.h"
#include "World.h"

namespace RayTracer
{
	class Scene : public IScene
	{
	private:
		std::vector<IIntersectable*> objects;
		std::vector<IMaterial*> materials;
		const RayTracer::World world;
	public:
		virtual const std::vector<IIntersectable*> &Objects() const override
		{
			return objects;
		}

		virtual const std::vector<IMaterial *> &Materials() const override
		{
			return materials;
		}

		virtual const IWorld *World() const
		{
			return &world;
		}

		void AddObject(IIntersectable* object)
		{
			objects.emplace_back(object);
		}

		void AddMaterial(IMaterial *material)
		{
			materials.emplace_back(material);
		}
	};
}