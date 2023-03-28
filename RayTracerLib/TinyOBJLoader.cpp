#include "TinyOBJLoader.h"
#include "Vector3.h"
#include "Mesh.h"
#include "DiffuseBSDF.h"
#include "Scene.h"
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#undef TINYOBJLOADER_IMPLEMENTATION

using RayTracer::ImageResolution;
using RayTracer::Camera;
using RayTracer::IScene;
using RayTracer::Scene;
using RayTracer::Vector3;
using RayTracer::Mesh;
using RayTracer::DiffuseBSDF;

using tinyobj::ObjReader;

// TODO: This needs to read in the full set of data from the obj,
// and make materials from the mtl files, and have face normals,
// and, and, and etc...
bool RayTracer::CreateSceneFromOBJFile(const std::string &file_path, ImageResolution resolution, Camera *&out_camera, IScene *&out_scene)
{
	ObjReader reader;
	if (!reader.ParseFromFile(file_path))
	{
		return false;
	}

	std::vector<Mesh*> meshes;

	for (const auto &shape : reader.GetShapes())
	{
		std::vector<Vector3<float>> mesh_vertices;
		std::vector<Vector3<size_t>> mesh_vertex_indicies;

		const std::vector<tinyobj::index_t> &indices = shape.mesh.indices;
		mesh_vertex_indicies.reserve(indices.size());

		const std::vector<tinyobj::real_t> &vertices = reader.GetAttrib().vertices;

		for (const auto &index : indices)
		{
			Vector3<float> vertex(vertices[3 * index.vertex_index], vertices[3 * index.vertex_index + 1], vertices[3 * index.vertex_index + 2]);

			mesh_vertices.emplace_back(vertex);
		}

		for (size_t i = 0; i < mesh_vertices.size() / 3; i++)
		{
			mesh_vertex_indicies.emplace_back(Vector3<size_t>(3 * i, 3 * i + 1, 3 * i + 2));
		}

		Mesh *mesh = new Mesh(std::make_shared<const DiffuseBSDF>(Color(1.0f, 0, 0, 1), 0.3f), mesh_vertices, mesh_vertex_indicies);
		meshes.emplace_back(mesh);
	}

	out_camera = new Camera(resolution, Vector3<float>(-5, 5, 5), Vector3<float>(1, -1, -1), 50, 18);
	
	Scene *scene = new Scene();

	for (const auto &mesh : meshes)
	{
		scene->AddObject(mesh);
	}

	out_scene = scene;
	return true;
}


