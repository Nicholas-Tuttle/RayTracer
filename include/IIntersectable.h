#pragma once

#include "Vector3.h"
#include "Color.h"

namespace RayTracer
{
	class IRay
	{
	public:
		virtual const Vector3<float> &Origin() const = 0;
		virtual const Vector3<float> &Direction() const = 0;
	};

	class IIntersection
	{
	public:
		virtual float Depth() const = 0;
		virtual const Vector3<float> &Location() const = 0;
		virtual const Vector3<float> &Normal() const = 0;
	};

	class IIntersectable
	{
	public:
		virtual bool IntersectsRay(const std::unique_ptr<IRay> &incoming_ray, std::unique_ptr<IIntersection> &out_intersection_info) const = 0;
		virtual int MaterialIndex() const = 0;
	};
}
