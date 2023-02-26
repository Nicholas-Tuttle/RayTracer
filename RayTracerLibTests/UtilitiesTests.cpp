#include "gtest/gtest.h"
#include "Utilities.h"

using RayTracer::Lerp;

namespace LerpTests
{
	TEST(LerpTests, LerpTest_01)
	{
		unsigned int test_count = 100;
		for (unsigned int i = 0; i < test_count; i++)
		{
			float float_i = ((float)i) / test_count;
			ASSERT_EQ(float_i, Lerp<float>(0, 1, float_i));
		}
	}

	TEST(LerpTests, LerpTest_min)
	{
		ASSERT_EQ(0, Lerp<float>(0, 1, -10.0f));
	}

	TEST(LerpTests, LerpTest_max)
	{
		ASSERT_EQ(1, Lerp<float>(0, 1, 10.0f));
	}
}