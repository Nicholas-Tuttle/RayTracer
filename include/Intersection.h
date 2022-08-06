#pragma once

#include "IIntersectable.h"

namespace RayTracer
{
	class Intersection : public IIntersection
	{
	private:
		float depth;
		Vector3<float> normal;
		Vector3<float> location;
	public:
		Intersection(float depth, Vector3<float> normal, Vector3<float> location)
			: depth(depth), normal(normal), location(location) {};

		virtual float Depth() const 
		{ 
			return depth; 
		}

		virtual Vector3<float> Location() const
		{
			return location;
		}

		virtual Vector3<float> Normal() const
		{
			return normal;
		}
	};
}
