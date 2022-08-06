#pragma once

#include <math.h>
#include "Utilities.h"

namespace RayTracer
{
	template <class T>
	class Vector2
	{
	public:
		Vector2(T x, T y)
			: X(x), Y(y) {};
		const T X;
		const T Y;
	};

	template <class T>
	class Vector3
	{
	public:
		Vector3() : Vector3<T>((T)1.0, (T)1.0, (T)1.0) {};
		Vector3(T x, T y, T z) : X(x), Y(y), Z(z) {};
		T X;
		T Y;
		T Z;

		inline T Dot(const Vector3<T> vector) const
		{
			return X * vector.X + Y * vector.Y + Z * vector.Z;
		}

		inline Vector3<T> operator-(const Vector3<T> vector) const
		{
			return Vector3<T>(X - vector.X, Y - vector.Y, Z - vector.Z);
		}

		inline Vector3<T> operator+(const Vector3<T> vector) const
		{
			return Vector3<T>(X + vector.X, Y + vector.Y, Z + vector.Z);
		}

		void operator=(const Vector3<T>& vector)
		{
			X = vector.X;
			Y = vector.Y;
			Z = vector.Z;
		}

		Vector3<T> operator*(const double scalar) const
		{
			return Vector3<T>((T)(X * scalar), (T)(Y * scalar), (T)(Z * scalar));
		}

		inline bool operator==(const Vector3<T>& vector) const
		{
			return vector.X == X && vector.Y == Y && vector.Z == Z;
		}

		inline T MagnitudeSquared() const
		{
			return Dot(*this);
		}

		inline Vector3<T> Normalize() const
		{
			double magnitude = sqrt(MagnitudeSquared());

			return Vector3<T>((T)(X / magnitude), (T)(Y / magnitude), (T)(Z / magnitude));
		}

		Vector3<T> Cross(const Vector3<T> other) const
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
	};


	template <class T>
	class Vector4
	{
	public:
		Vector4()
			: W(1), X(1), Y(1), Z(1) {};
		Vector4(T w, T x, T y, T z)
			: W(w), X(x), Y(y), Z(z) {};
		T W;
		T X;
		T Y;
		T Z;

		inline T Dot(const Vector4<T> vector) const
		{
			return W * vector.W + X * vector.X + Y * vector.Y + Z * vector.Z;
		}

		inline Vector4<T> operator-(const Vector4<T> vector) const
		{
			return Vector4<T>(W - vector.W, X - vector.X, Y - vector.Y, Z - vector.Z);
		}

		inline T MagnitudeSquared() const
		{
			return W * W + X * X + Y * Y + Z * Z;
		}

		inline Vector4<T> Normalize() const
		{
			double magnitude = sqrt(MagnitudeSquared());

			return Vector4<T>((T)(W / magnitude), (T)(X / magnitude), (T)(Y / magnitude), (T)(Z / magnitude));
		}
	};
}