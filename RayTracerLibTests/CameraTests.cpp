
#include "gtest/gtest.h"
#include <Camera.h>
#include <Vector3.h>

using RayTracer::Vector3;
using RayTracer::Camera;
using RayTracer::ImageResolution;

namespace CameraTests
{
	TEST(CameraTests, CameraRayTest_01)
	{
		ImageResolution resolution(4, 4);
		Camera camera(resolution, Vector3<float>(0, 0, 0), Vector3<float>(1, 0, 0), 1000, 1000);

		auto pixels = camera.GetOutgoingPixels();

		ASSERT_EQ(16, pixels.size());

		Vector3<float> expected[16] =
		{
			Vector3<float>(1, 0.75, -0.75), Vector3<float>(1, 0.75, -0.25), Vector3<float>(1, 0.75, 0.25), Vector3<float>(1, 0.75, 0.75),
			Vector3<float>(1, 0.25, -0.75), Vector3<float>(1, 0.25, -0.25), Vector3<float>(1, 0.25, 0.25), Vector3<float>(1, 0.25, 0.75),
			Vector3<float>(1, -0.25, -0.75), Vector3<float>(1, -0.25, -0.25), Vector3<float>(1, -0.25, 0.25), Vector3<float>(1, -0.25, 0.75),
			Vector3<float>(1, -0.75, -0.75), Vector3<float>(1, -0.75, -0.25), Vector3<float>(1, -0.75, 0.25), Vector3<float>(1, -0.75, 0.75),
		};

		for (int i = 0; i < 16; i++)
		{
			ASSERT_EQ(expected[i].X, pixels[i].CentralRayDirection().X);
			ASSERT_EQ(expected[i].Y, pixels[i].CentralRayDirection().Y);
			ASSERT_EQ(expected[i].Z, pixels[i].CentralRayDirection().Z);
		}
	}
}