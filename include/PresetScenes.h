#pragma once

#include <IScene.h>
#include <Camera.h>
#include <Scene.h>
#include <Material.h>
#include <EmissiveBSDF.h>
#include <GlossyBSDF.h>
#include <DiffuseBSDF.h>
#include <Sphere.h>

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

	static void CreatePresetScene1(std::unique_ptr<IScene> &out_scene, std::unique_ptr<Camera> &camera, ImageResolution resolution)
	{
        camera = std::unique_ptr<Camera>(new Camera(resolution, Vector3<float>(0, 0.25f, 0), Vector3<float>(1, 0, 0), 50, 18));

        // Build a scene to render
        Scene *scene = new Scene();

        scene->AddMaterial(new GlossyBSDF(Color(0.8f, 0.8f, 0.8f, 1.0f), 0.2f));
        scene->AddMaterial(new GlossyBSDF(Color(0.8f, 0.8f, 0.8f, 1.0f), 0.3f));
        scene->AddMaterial(new DiffuseBSDF(Color(1.0f, 0.5f, 0.5f, 1.0f), RandomRoughness(0.0f, 0.3f)));
        scene->AddMaterial(new DiffuseBSDF(Color(0.5f, 1.0f, 0.5f, 1.0f), RandomRoughness(0.0f, 0.3f)));
        scene->AddMaterial(new DiffuseBSDF(Color(0.5f, 0.5f, 1.0f, 1.0f), RandomRoughness(0.0f, 0.3f)));
        scene->AddMaterial(new DiffuseBSDF(RandomColor(), RandomRoughness(0.0f, 0.3f)));
        scene->AddMaterial(new EmissiveBSDF(Color(1.0f, 0.0f, 1.0f, 1.0f)));

        scene->AddObject(new Sphere(Vector3<float>(  9,    0,    -3),   1.0f,   0));
        scene->AddObject(new Sphere(Vector3<float>( 14,    1,    -3),   1.0f,   1));
        scene->AddObject(new Sphere(Vector3<float>( 10,   -1,     0),   1.0f,   2));
        scene->AddObject(new Sphere(Vector3<float>( 14,    0,     1),   1.0f,   3));
        scene->AddObject(new Sphere(Vector3<float>( 18,    1,     2),   1.0f,   4));
        scene->AddObject(new Sphere(Vector3<float>( 16,   -1,     4),   1.0f,   5));
        scene->AddObject(new Sphere(Vector3<float>( 12,    0,     4),   1.0f,   6));

        out_scene = std::unique_ptr<IScene>(scene);
	}

	static void CreatePresetSceneSphereArray(std::unique_ptr<IScene> &out_scene, std::unique_ptr<Camera> &camera, ImageResolution resolution)
	{
        float sphere_array_count = 15;

        camera = std::unique_ptr<Camera>(new Camera(resolution, 
            Vector3<float>((sphere_array_count - 1) / 2, sphere_array_count * 2.5f, (sphere_array_count - 1) / 2),
            Vector3<float>(0, -1, 0), 50, 18));

        // Build a scene to render
        Scene *scene = new Scene(); 
        scene->AddMaterial(new DiffuseBSDF(Color(1.0f, 1.0f, 1.0f, 1.0f), 0.1f));

        for (float i = 0; i < sphere_array_count; i++)
        {
            for (float j = 0; j < sphere_array_count; j++)
            {
                scene->AddObject(new Sphere(Vector3<float>(i, 0, j), 0.4f, 0));
            }
        }

        out_scene = std::unique_ptr<IScene>(scene);
	}
}
