#include "gtest/gtest.h"
#include "IIntersectable.h"
#include "Intersection.h"
#include "IMaterial.h"
#include "Mesh.h"
#include "Color.h"

using RayTracer::Mesh;
using RayTracer::Intersection;
using RayTracer::Vector3;
using RayTracer::Ray;
using RayTracer::Color;

namespace MeshTests
{
	TEST(MeshTests, MeshIntersectionTest_01)
	{
		Mesh mesh(std::shared_ptr<RayTracer::IMaterial>(nullptr),
			{
				Vector3<float>(0.0f, 0.0f, 1.0f),
				Vector3<float>(1.0f, 0.0f, 1.0f),
				Vector3<float>(0.0f, 1.0f, 1.0f),
			},
			{
				Vector3<size_t>(0, 1, 2)
			});

		Ray ray(Vector3<float>(0.1f, 0.1f, 0.0f), Vector3<float>(0.0f, 0.0f, 1.0f), Color());
		Intersection intersection;

		bool intersects = mesh.IntersectsRay(ray, intersection);

		ASSERT_TRUE(intersects);
		ASSERT_FLOAT_EQ(intersection.Depth(), 1.0f);
	}

	TEST(MeshTests, MeshIntersectionTest_02)
	{
		Mesh mesh(std::shared_ptr<RayTracer::IMaterial>(nullptr),
			{
				Vector3<float>(0.0f, 0.0f, 1.0f),
				Vector3<float>(1.0f, 0.0f, 1.0f),
				Vector3<float>(0.0f, 1.0f, 1.0f),
			},
			{
				Vector3<size_t>(0, 1, 2)
			});

		Ray ray(Vector3<float>(0.6f, 0.6f, 0.0f), Vector3<float>(0.0f, 0.0f, 1.0f), Color());
		Intersection intersection;

		bool intersects = mesh.IntersectsRay(ray, intersection);

		ASSERT_FALSE(intersects);
	}

	TEST(MeshTests, MeshIntersectionTest_03)
	{
		Mesh mesh(std::shared_ptr<RayTracer::IMaterial>(nullptr),
			{
				Vector3<float>(0.0f, 0.0f, 1.0f),
				Vector3<float>(1.0f, 0.0f, 1.0f),
				Vector3<float>(0.0f, 1.0f, 1.0f),
				Vector3<float>(1.0f, 1.0f, 1.0f),
			},
			{
				Vector3<size_t>(0, 1, 2),
				Vector3<size_t>(2, 1, 3)
			});

		Ray ray(Vector3<float>(0.6f, 0.6f, 0.0f), Vector3<float>(0.0f, 0.0f, 1.0f), Color());
		Intersection intersection;

		bool intersects = mesh.IntersectsRay(ray, intersection);

		ASSERT_TRUE(intersects);
		ASSERT_FLOAT_EQ(intersection.Depth(), 1.0f);
	}
}

