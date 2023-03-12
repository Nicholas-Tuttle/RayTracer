#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <chrono>

namespace RayTracer::PerformanceTracking
{
	class PerformanceSection;

	class PerformanceSession
	{
	public:
		friend class PerformanceSection;

		enum class PerformanceTrackingGranularity
		{
			ExtremelyCoarse,
			VeryCoarse,
			Coarse,
			Average,
			Granular,
			VeryGranular,
			ExtremelyGranular
		};

		PerformanceSession(PerformanceTrackingGranularity granularity, std::string &&output_filename)
			: output_filename(std::move(output_filename))
		{
			messages.reserve(size_t(32) << static_cast<size_t>(granularity));
		}

		~PerformanceSession()
		{
			std::ofstream stream(output_filename);
			stream << "{\n\t\"traceEvents\": [\n";
			for (size_t i = 0; i < messages.size(); i++)
			{
				stream << "\t\t" << messages[i];
				if (i != messages.size() - 1)
				{
					stream << ",";
				}
				stream << "\n";
			}
			stream << "\t]\n}";
			stream.flush();
			stream.close();
		}

	private:
		void AddPerformanceEntry(std::string &&message)
		{
			messages.emplace_back(std::move(message));
		}

		std::string output_filename;
		std::vector<std::string> messages;
	};

	class PerformanceSection
	{
	public:
		PerformanceSection(PerformanceSession *const session, const char *name)
			: session(session), name(name)
		{
			if (session)
			{
				long long start = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
				session->AddPerformanceEntry(std::move("{\"name\": \"" + this->name + "\", \"cat\": \"PERF\", \"ph\": \"B\", \"pid\" : 0, \"ts\": " + std::to_string(start) + "}"));
			}
		}

		~PerformanceSection()
		{
			if (session)
			{
				long long end = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
				session->AddPerformanceEntry(std::move("{\"name\": \"" + this->name + "\", \"cat\": \"PERF\", \"ph\": \"E\", \"pid\" : 0, \"ts\": " + std::to_string(end) + "}"));
			}
		}

	private:
		PerformanceSession *const session;
		const std::string name;
	};
}

#define TRACE_SCOPE(performance_session, name) RayTracer::PerformanceTracking::PerformanceSection perf_section_ ## name(performance_session, # name)
#define TRACE_FUNCTION(performance_session) RayTracer::PerformanceTracking::PerformanceSection perf_function(performance_session, __FUNCTION__)

