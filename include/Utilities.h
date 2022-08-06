#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <fstream>

namespace RayTracer
{
	template<class T>
	static T Lerp(T first, T second, float lerp_value)
	{
		lerp_value = std::min<T>(std::max<T>((T)0, lerp_value), (T)1);
		return (first * (1.0f - lerp_value)) + (second * lerp_value);
	}

    static std::vector<uint8_t> ReadShaderFileToBytes(const std::string &filename)
    {
        std::ifstream file;
        file.open(filename, std::ifstream::in | std::ifstream::ate | std::ifstream::binary);

        if (!file.is_open() || !file.good()) {
            return std::vector<uint8_t>();
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<uint8_t> buffer(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char *>(&buffer[0]), fileSize);
        file.close();

        return buffer;
    }

}
