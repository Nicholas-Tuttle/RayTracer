#pragma once

#include "tiny_obj_loader.h"
#include "Image.h"
#include "Camera.h"
#include "IScene.h"
#include <string>

namespace RayTracer
{
	bool CreateSceneFromOBJFile(const std::string &file_path, ImageResolution resolution, Camera *&out_camera, IScene *&out_scene);
}

