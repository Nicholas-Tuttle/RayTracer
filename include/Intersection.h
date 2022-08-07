#pragma once

#include "IIntersectable.h"

namespace RayTracer
{
	class Intersection : public IIntersection
	{
	private:
		const float depth;
		const Vector3<float> normal;
		const Vector3<float> location;
	public:
		Intersection(float depth, const Vector3<float> &normal, const Vector3<float> &location)
			: depth(depth), normal(normal), location(location) {};

		virtual float Depth() const override
		{ 
			return depth; 
		}

		virtual const Vector3<float> &Location() const override
		{
			return location;
		}

		virtual const Vector3<float> &Normal() const override
		{
			return normal;
		}
	};
}
