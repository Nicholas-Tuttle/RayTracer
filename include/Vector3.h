#pragma once

#include "Utilities.h"

#include <math.h>

namespace RayTracer
{
	template <class T>
	class Vector3
	{
	public:
		Vector3() : Vector3<T>((T)1.0, (T)1.0, (T)1.0) {};
		Vector3(const T &x, const T &y, const T &z) : X(x), Y(y), Z(z), 
			CalculatedNormalized(false), XNormalized(T()), YNormalized(T()), ZNormalized(T()) {};
		T X;
		T Y;
		T Z;

		inline T Dot(const Vector3<T> &vector) const
		{
			return X * vector.X + Y * vector.Y + Z * vector.Z;
		}

		inline Vector3<T> operator-(const Vector3<T> &vector) const
		{
			return Vector3<T>(X - vector.X, Y - vector.Y, Z - vector.Z);
		}

		inline Vector3<T> operator+(const Vector3<T> &vector) const
		{
			return Vector3<T>(X + vector.X, Y + vector.Y, Z + vector.Z);
		}

		void operator=(const Vector3<T> &vector)
		{
			X = vector.X;
			Y = vector.Y;
			Z = vector.Z;
			CalculatedNormalized = vector.CalculatedNormalized;
			XNormalized = vector.XNormalized;
			YNormalized = vector.YNormalized;
			ZNormalized = vector.ZNormalized;
		}

		Vector3<T> operator*(const double scalar) const
		{
			return Vector3<T>((T)(X * scalar), (T)(Y * scalar), (T)(Z * scalar));
		}

		inline bool operator==(const Vector3<T> &vector) const
		{
			return vector.X == X && vector.Y == Y && vector.Z == Z;
		}

		inline bool operator!=(const Vector3<T> &vector) const
		{
			return vector.X != X || vector.Y != Y || vector.Z != Z;
		}

		inline T operator[](int i) const
		{
			switch (i)
			{
			case 0:
				return X;
			case 1:
				return Y;
			case 2:
				return Z;
			default:
				return T();
			}
		}

		inline T MagnitudeSquared() const
		{
			return Dot(*this);
		}

		inline Vector3<T> Normalize() const
		{
			if (CalculatedNormalized)
			{
				return Vector3<T>(XNormalized, YNormalized, ZNormalized);
			}
			else
			{
				T magnitude = sqrt(MagnitudeSquared());
				XNormalized = X / magnitude;
				YNormalized = Y / magnitude;
				ZNormalized = Z / magnitude;
				CalculatedNormalized = true;
				return Vector3<T>(XNormalized, YNormalized, ZNormalized);
			}
		}

		Vector3<T> Cross(const Vector3<T> &other) const
		{
			return Vector3<T>(
				((Y * other.Z) - (Z * other.Y)),
				((Z * other.X) - (X * other.Z)),
				((X * other.Y) - (Y * other.X)));
		}

		Vector3<T> NormalizedLerp(const Vector3<T> &other, float lerp_value)
		{
			T x_lerp = Lerp<T>(X, other.X, lerp_value);
			T y_lerp = Lerp<T>(Y, other.Y, lerp_value);
			T z_lerp = Lerp<T>(Z, other.Z, lerp_value);

			return Vector3<T>(x_lerp, y_lerp, z_lerp).Normalize();
		}

		// Based on https://raytracing.github.io/books/RayTracingInOneWeekend.html#diffusematerials/asimplediffusematerial
		static Vector3<T> RandomInUnitSphere()
		{
			while (true)
			{
				T x = (T)rand() / RAND_MAX * 2 - 1;
				T y = (T)rand() / RAND_MAX * 2 - 1;
				T z = (T)rand() / RAND_MAX * 2 - 1;
				Vector3<T> vec(x, y, z);
				if (vec.MagnitudeSquared() < 1)
				{
					return vec;
				}
			}
		}

	private:
		mutable bool CalculatedNormalized;
		mutable T XNormalized;
		mutable T YNormalized;
		mutable T ZNormalized;
	};
}