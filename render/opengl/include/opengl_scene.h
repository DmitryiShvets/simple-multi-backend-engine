#pragma once
#include <map>

#include "opengl_renderer.h"

class Scene
{
public:
	void AddObjectToScene(RenderObject robject);
	void DrawAllObjects();

private:
	std::map<std::string, RenderObject> objects;
};
