#pragma once

#include "Vector3.h"

namespace RayTracer
{
	class Intersection
	{
	private:
		float depth;
		Vector3<float> normal;
		Vector3<float> location;
	public:
		Intersection() : depth(0), normal(), location() {};
		Intersection(float depth, const Vector3<float> &normal, const Vector3<float> &location)
			: depth(depth), normal(normal), location(location) {};

		virtual float Depth() const
		{ 
			return depth; 
		}

		virtual const Vector3<float> &Location() const
		{
			return location;
		}

		virtual const Vector3<float> &Normal() const
		{
			return normal;
		}

		void operator=(const Intersection &other)
		{
			depth = other.depth;
			normal = other.normal;
			location = other.location;
		}
	};
}
