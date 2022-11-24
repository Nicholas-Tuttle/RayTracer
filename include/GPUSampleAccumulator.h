#pragma once

namespace RayTracer
{
	class GPUSampleAccumulator
	{
	public:
		void AccumulateSample() const
		{
			std::cout << __FUNCTION__ << std::endl;
		}
	};
}
