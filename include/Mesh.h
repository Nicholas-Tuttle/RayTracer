#pragma once

#include "IIntersectable.h"
#include "Vector3.h"

namespace RayTracer
{
	class Mesh : public IIntersectable
	{
	public:
		Mesh(std::shared_ptr<const IMaterial> material, const std::vector<Vector3<float>> &vertices, const std::vector<Vector3<size_t>> &face_vertex_indices)
			: material(material), VertexData(vertices), VertexIndices(face_vertex_indices)
		{
			for (const auto &indicies : face_vertex_indices)
			{
				if (indicies.X >= vertices.size() || indicies.Y == vertices.size() || indicies.Z == vertices.size())
				{
					throw std::exception("Face vertex indices are out-of-bounds of provided vertices");
				}

				faces.emplace_back(MeshTriangleFace(vertices[indicies.X], vertices[indicies.Y], vertices[indicies.Z]));
			}
		}

		virtual bool IntersectsRay(const Ray &incoming_ray, Intersection &out_intersection_info) const
		{
			bool intersection_found = false;
			Intersection final_intersection;

			for (const auto &face : faces)
			{
				Intersection temp_intersection;
				if (face.IntersectsRay(incoming_ray, temp_intersection) && temp_intersection.Depth() < final_intersection.Depth())
				{
					intersection_found = true;
					final_intersection = temp_intersection;
				}
			}

			out_intersection_info = final_intersection;
			return intersection_found;
		}

		virtual const std::shared_ptr<const IMaterial> Material() const
		{
			return material;
		}

		const std::vector<Vector3<float>> VertexData;
		std::vector<Vector3<size_t>> VertexIndices;

	private:
		// TODO: move material to individual faces
		std::shared_ptr<const IMaterial> material;

		class MeshTriangleFace
		{
		public:
			MeshTriangleFace(Vector3<float> vertex_1, Vector3<float> vertex_2, Vector3<float> vertex_3)
			{
				vertices[0] = MeshVertex(vertex_1);
				vertices[1] = MeshVertex(vertex_2);
				vertices[2] = MeshVertex(vertex_3);
			}

			// See https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
			bool IntersectsRay(const Ray &incoming_ray, Intersection &out_intersection_info) const
			{
				Vector3<float> edge_1 = vertices[1].position - vertices[0].position;
				Vector3<float> edge_2 = vertices[2].position - vertices[0].position;

				Vector3<float> ray_direction_normalized = incoming_ray.Direction().Normalize();
				Vector3<float> h = ray_direction_normalized.Cross(edge_2);
				float a = edge_1.Dot(h);

				if (0 == a)
				{
					// The ray is parallel to the triangle
					return false;
				}

				float f = 1.0f / a;
				Vector3<float> s = incoming_ray.Origin() - vertices[0].position;
				float u = f * s.Dot(h);

				if (0.0f > u || 1.0f < u)
				{
					return false;
				}

				Vector3<float> q = s.Cross(edge_1);
				float v = f * ray_direction_normalized.Dot(q);

				if (0.0f > v || 1.0f < (u + v))
				{
					return false;
				}

				// Find where the intersection point lies on the line
				float t = f * edge_2.Dot(q);

				// If "t" is negative there is a line intersection but not a ray
				// (the intersection is behind the ray)
				if (0 < t)
				{
					Intersection new_intersection(sqrt((ray_direction_normalized * t).MagnitudeSquared()), edge_1.Cross(edge_2), incoming_ray.Origin() + ray_direction_normalized * t);
					out_intersection_info = new_intersection;
					return true;
				}

				return false;
			}

		private:
			struct MeshVertex
			{
				Vector3<float> position;

				explicit MeshVertex(Vector3<float> position) : position(position) {}
				MeshVertex() {};
			};

			MeshVertex vertices[3];
		};

		std::vector<MeshTriangleFace> faces;
	};
}
