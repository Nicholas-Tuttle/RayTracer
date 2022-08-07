#pragma once

#include <math.h>
#include "Utilities.h"

namespace RayTracer
{
	template <class T>
	class Vector3
	{
	public:
		Vector3() : Vector3<T>((T)1.0, (T)1.0, (T)1.0) {};
		Vector3(const T &x, const T &y, const T &z) : X(x), Y(y), Z(z), 
			CalculatedNormalized(false), XNormalized(0), YNormalized(0), ZNormalized(0) {};
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

	private:
		mutable bool CalculatedNormalized;
		mutable T XNormalized;
		mutable T YNormalized;
		mutable T ZNormalized;
	};
}