#pragma once

#include "IScene.h"
#include "Camera.h"
#include "Scene.h"
#include "EmissiveBSDF.h"
#include "GlossyBSDF.h"
#include "DiffuseBSDF.h"
#include "Sphere.h"
#include "Mesh.h"

namespace RayTracer
{
    static Color RandomColor()
    {
        float random1 = (float)rand() / RAND_MAX;
        float random2 = (float)rand() / RAND_MAX;
        float random3 = (float)rand() / RAND_MAX;
        return Color(random1, random2, random3, 1.0f);
    }

    static float RandomRoughness(float min, float max)
    {
        return RayTracer::Lerp<float>(min, max, (float)rand() / RAND_MAX);
    }

	static void CreatePresetScene1(IScene *&out_scene, Camera *&out_camera, ImageResolution resolution)
	{
        out_camera = new Camera(resolution, Vector3<float>(0, 0.25f, 0), Vector3<float>(1, 0, 0), 50, 18);

        Scene *scene = new Scene();
        scene->AddObject(new Sphere(Vector3<float>(  9,    0,    -3),   1.0f,   
            std::make_shared<const DiffuseBSDF>(RandomColor(), RandomRoughness(0.0f, 0.3f))));
        scene->AddObject(new Sphere(Vector3<float>( 14,    1,    -3),   1.0f, 
            std::make_shared<const DiffuseBSDF>(RandomColor(), RandomRoughness(0.0f, 0.3f))));
        scene->AddObject(new Sphere(Vector3<float>( 10,   -1,     0),   1.0f,   
            std::make_shared<const DiffuseBSDF>(RandomColor(), RandomRoughness(0.0f, 0.3f))));
        scene->AddObject(new Sphere(Vector3<float>( 14,    0,     1),   1.0f,   
            std::make_shared<const DiffuseBSDF>(RandomColor(), RandomRoughness(0.0f, 0.3f))));
        scene->AddObject(new Sphere(Vector3<float>( 18,    1,     2),   1.0f,   
            std::make_shared<const DiffuseBSDF>(RandomColor(), RandomRoughness(0.0f, 0.3f))));
        scene->AddObject(new Sphere(Vector3<float>( 16,   -1,     4),   1.0f,   
            std::make_shared<const DiffuseBSDF>(RandomColor(), RandomRoughness(0.0f, 0.3f))));
        scene->AddObject(new Sphere(Vector3<float>( 12,    0,     4),   1.0f,   
            std::make_shared<const EmissiveBSDF>(RandomColor(), 10.0f)));
        scene->AddObject(new Sphere(Vector3<float>(0, -1002, 0), 1000.0f,
            std::make_shared<const DiffuseBSDF>(Color(0.5f, 0.5f, 0.5f, 1.0f), 0.5f)));
        out_scene = scene;
	}

    static void CreatePresetScene1Sphere1Light(IScene *&out_scene, Camera *&out_camera, ImageResolution resolution)
    {
        out_camera = new Camera(resolution, Vector3<float>(0, 0, 0), Vector3<float>(1, 0, 0), 50, 18);

        Scene *scene = new Scene();
        scene->AddObject(new Sphere(Vector3<float>(5, 0, -1), 0.5f, 
            std::make_shared<const DiffuseBSDF>(Color(1.0f, 0, 1.0f, 1.0f), 0.3f)));
        scene->AddObject(new Sphere(Vector3<float>(5, 0, 1), 0.1f, 
            std::make_shared<const EmissiveBSDF>(Color(0.0f, 0.0f, 1.0f, 1.0f), 100.0f)));
        out_scene = scene;
    }

	static void CreatePresetSceneSphereArray(IScene *&out_scene, Camera *&out_camera, ImageResolution resolution)
	{
        float sphere_array_count = 15;
        
        out_camera = new Camera(resolution, Vector3<float>((sphere_array_count - 1) / 2, sphere_array_count * 2.5f, (sphere_array_count - 1) / 2), Vector3<float>(0, -1, 0), 50, 18);
        
        Scene *scene = new Scene();
        for (float i = 0; i < sphere_array_count; i++)
        {
            for (float j = 0; j < sphere_array_count; j++)
            {
                scene->AddObject(new Sphere(Vector3<float>(i, 0, j), 0.4f, 
                    std::make_shared<const EmissiveBSDF>(RandomColor(), 5.0f)));
            }
        }
        out_scene = scene;
	}

    static void CreatePresetSceneCube(IScene *&out_scene, Camera *&out_camera, ImageResolution resolution)
    {
        out_camera = new Camera(resolution, Vector3<float>(-1, 1, 1), Vector3<float>(1, -1, -1), 50, 18);
        Scene *scene = new Scene();

        // This is a cube
        scene->AddObject(new Mesh(std::make_shared<const DiffuseBSDF>(Color(1.0f, 0, 0, 1), RandomRoughness(0.0f, 0.3f)),
            {
                Vector3<float>(-0.25f, -0.25f, 0.25f), //  [0] : bottom-left front
                Vector3<float>(0.25f, -0.25f, 0.25f), //   [1] : bottom-right front
                Vector3<float>(-0.25f, 0.25f, 0.25f), //   [2] : top-left front
                Vector3<float>(0.25f, 0.25f, 0.25f), //    [3] : top-right front
                Vector3<float>(-0.25f, -0.25f, -0.25f), // [4] : bottom-left back
                Vector3<float>(0.25f, -0.25f, -0.25f), //  [5] : bottom-right back
                Vector3<float>(-0.25f, 0.25f, -0.25f), //  [6] : top-left back
                Vector3<float>(0.25f, 0.25f, -0.25f), //   [7] : top-right back
            },
            {
                Vector3<size_t>(0, 1, 2), // bottom-front
                Vector3<size_t>(2, 1, 3), // top-front
                Vector3<size_t>(4, 0, 2), // bottom-left
                Vector3<size_t>(2, 6, 4), // top-left
                Vector3<size_t>(5, 3, 1), // bottom-right
                Vector3<size_t>(5, 7, 3), // top-right
                Vector3<size_t>(5, 4, 6), // bottom-back
                Vector3<size_t>(6, 7, 5), // top-back
                Vector3<size_t>(6, 2, 3), // front-top
                Vector3<size_t>(3, 7, 6), // back-top
                Vector3<size_t>(1, 0, 4), // front-bottom
                Vector3<size_t>(4, 5, 1), // back-bottom
            }));
        // Add a sphere inside the cube (that won't be seen)
        scene->AddObject(new Sphere(Vector3<float>(), 0.2f, std::make_shared<const EmissiveBSDF>(RandomColor(), 10.0f)));
        // Add a sphere behind the cube
        scene->AddObject(new Sphere(Vector3<float>(0.5f, 0, 0), 0.2f, std::make_shared<const EmissiveBSDF>(Color(0, 1.0f, 0, 1), 1.0f)));
        // Add a sphere to the side of the cube
        scene->AddObject(new Sphere(Vector3<float>(0, 0, 0.5f), 0.2f, std::make_shared<const EmissiveBSDF>(Color(0, 0, 1.0f, 1), 1.0f)));

        out_scene = scene;
    }
}
