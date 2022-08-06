#pragma once
#include "Pixel.h"
#include "Image.h"

namespace RayTracer
{
	class Camera
	{
	private:
		static const int MM_in_M = 1000;

		const ImageResolution resolution;
		Vector3<float> position;
		Vector3<float> forwardVector;
		Vector3<float> rightVector;
		Vector3<float> upVector;
		const float focalLengthMM;
		const float sensorWidthMM;
	public:
		Camera(ImageResolution resolution, Vector3<float> position, Vector3<float> forwardVector, float focalLength, float sensorWidthMM) :
			resolution(resolution), position(position), forwardVector(forwardVector.Normalize()), 
			focalLengthMM(focalLength), sensorWidthMM(sensorWidthMM)
		{
			Vector3<float> camera_right_vector;
			Vector3<float> camera_up_vector;

			if (forwardVector == Vector3<float>(0, 1, 0))
			{
				rightVector = Vector3<float>(0, 0, 1);
				upVector = Vector3<float>(-1, 0, 0);
			}
			else if (forwardVector == Vector3<float>(0, -1, 0))
			{
				rightVector = Vector3<float>(0, 0, 1);
				upVector = Vector3<float>(1, 0, 0);
			}
			else
			{
				rightVector = forwardVector.Cross(Vector3<float>(0, 1, 0)).Normalize();
				upVector = rightVector.Cross(forwardVector).Normalize();
			}
		}

		Vector3<float> Position() const
		{
			return position;
		}

		Vector3<float> ForwardVector() const
		{
			return forwardVector;
		}

		Vector3<float> RightVector() const
		{
			return rightVector;
		}

		Vector3<float> UpVector() const
		{
			return upVector;
		}

		ImageResolution Resolution() const
		{
			return resolution;
		}

		std::vector<Pixel> GetOutgoingPixels() const
		{
			std::vector<Pixel> pixels;

			pixels.reserve(resolution.X * resolution.Y);

			const Vector3<float> center_of_near_clip_plane = Vector3<float>(position + (forwardVector * (focalLengthMM / MM_in_M)));

			// Take the center of the near clip plane and offset left/right and up/down
			float sensor_height_m = sensorWidthMM * resolution.Y / MM_in_M / resolution.X;
			float sensor_width_m = sensorWidthMM / MM_in_M;

			float pixel_size_m = sensorWidthMM / resolution.X / MM_in_M;

			for (int y = 0; y < resolution.Y; y++)
			{
				float vertical_scalar = (((float)(-1 * ((2 * y) + 1)) / resolution.Y) + 1) * sensor_height_m;

				for (int x = 0; x < resolution.X; x++)
				{
					float horizontal_scalar = (((float)((2 * x) + 1) / resolution.X) - 1) * sensor_width_m;

					Vector3<float> ray = center_of_near_clip_plane + (rightVector * horizontal_scalar) + (upVector * vertical_scalar) - position;

					pixels.emplace_back(Pixel(position, ray, x, y, pixel_size_m, pixel_size_m));
				}
			}

			return pixels;
		}
	};
}