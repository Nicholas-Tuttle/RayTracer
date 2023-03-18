#pragma once

#include "Vector3.h"

#include <limits>

namespace RayTracer
{
	class Intersection
	{
	private:
		float depth;
		Vector3<float> normal;
		Vector3<float> location;
	public:
#ifdef max
#define max_was_defined
#pragma push_macro("max") // This can conflict with some other "max" macros
#undef max
#endif
		Intersection() : depth(std::numeric_limits<float>::max()), normal(), location() {};
#ifdef max_was_defined
#pragma pop_macro("max")
#undef max_was_defined
#endif
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
