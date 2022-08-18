
#include "gtest/gtest.h"
#include <IIntersectable.h>
#include <Intersection.h>
#include <Sphere.h>
#include <Ray.h>
#include <Vector3.h>

using RayTracer::Sphere;
using RayTracer::Ray;
using RayTracer::Intersection;
using RayTracer::Vector3;
using RayTracer::Color;

namespace SphereTests
{
	TEST(SphereTests, SphereIntersectionTest_01) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		Ray ray = Ray(Vector3<float>(-2, 0, 0), Vector3<float>(1, 0, 0), Color());
		Intersection intersection;

		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(true, intersects);
	}

	TEST(SphereTests, SphereIntersectionTest_02) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		Ray ray = Ray(Vector3<float>(-2, 1, 0), Vector3<float>(1, 0, 0), Color());
		Intersection intersection;

		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(true, intersects);
	}

	TEST(SphereTests, SphereIntersectionTest_03) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		Ray ray = Ray(Vector3<float>(-2, 1.1F, 0), Vector3<float>(1, 0, 0), Color());
		Intersection intersection;

		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(false, intersects);
	}

	TEST(SphereTests, SphereIntersectionTest_04) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		Ray ray = Ray(Vector3<float>(-2, 2, 0), Vector3<float>(1, -1, 0), Color());
		Intersection intersection;

		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(true, intersects);
	}

	TEST(SphereTests, SphereIntersectionTest_05) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		Ray ray = Ray(Vector3<float>(0, 0, 0), Vector3<float>(0, 1, 0), Color());
		Intersection intersection;

		// Backface, no intersection
		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(false, intersects);
	}
}