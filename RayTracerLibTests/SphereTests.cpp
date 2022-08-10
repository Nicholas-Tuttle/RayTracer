
#include "gtest/gtest.h"
#include <IIntersectable.h>
#include <Sphere.h>
#include <Ray.h>
#include <Vector3.h>

using RayTracer::Sphere;
using RayTracer::Ray;
using RayTracer::IRay;
using RayTracer::IIntersection;
using RayTracer::Vector3;

namespace SphereTests
{
	TEST(SphereTests, SphereIntersectionTest_01) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		std::unique_ptr<IRay>ray = std::unique_ptr<IRay>(new Ray(Vector3<float>(-2, 0, 0), Vector3<float>(1, 0, 0)));
		std::unique_ptr<IIntersection> intersection = nullptr;

		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(true, intersects);
	}

	TEST(SphereTests, SphereIntersectionTest_02) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		std::unique_ptr<IRay>ray = std::unique_ptr<IRay>(new Ray(Vector3<float>(-2, 1, 0), Vector3<float>(1, 0, 0)));
		std::unique_ptr<IIntersection> intersection = nullptr;

		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(true, intersects);
	}

	TEST(SphereTests, SphereIntersectionTest_03) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		std::unique_ptr<IRay>ray = std::unique_ptr<IRay>(new Ray(Vector3<float>(-2, 1.1F, 0), Vector3<float>(1, 0, 0)));
		std::unique_ptr<IIntersection> intersection = nullptr;

		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(false, intersects);
	}

	TEST(SphereTests, SphereIntersectionTest_04) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		std::unique_ptr<IRay>ray = std::unique_ptr<IRay>(new Ray(Vector3<float>(-2, 2, 0), Vector3<float>(1, -1, 0)));
		std::unique_ptr<IIntersection> intersection = nullptr;

		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(true, intersects);
	}

	TEST(SphereTests, SphereIntersectionTest_05) {

		Sphere sphere(Vector3<float>(0, 0, 0), 1);
		std::unique_ptr<IRay>ray = std::unique_ptr<IRay>(new Ray(Vector3<float>(0, 0, 0), Vector3<float>(0, 1, 0)));
		std::unique_ptr<IIntersection> intersection = nullptr;

		// Backface, no intersection
		bool intersects = sphere.IntersectsRay(ray, intersection);
		ASSERT_EQ(false, intersects);
	}
}