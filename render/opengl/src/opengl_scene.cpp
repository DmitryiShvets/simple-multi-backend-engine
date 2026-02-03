#include "opengl_scene.h"
#include "opengl_renderer.h"

void Scene::AddObjectToScene(RenderObject robject)
{
	objects.emplace(robject.uuid, std::move(robject));
}

void Scene::DrawAllObjects()
{
	//OpenGLRenderer::clear();

	//OpenglResourceManager* resources = &OpenglResourceManager::getInstance();
	//for (auto & entry : objects) {
	//	auto& [uuid, obj] = entry;
	//	ShaderProgram* mProgram = &resources->getProgram("custom");
	//	mProgram->use();
	//	mProgram->setUniform("customColor", glm::vec4(obj.color, 1.0));
	//	OpenGLRenderer::draw(obj.vao);
	//	mProgram->unbind();
	//}
}
