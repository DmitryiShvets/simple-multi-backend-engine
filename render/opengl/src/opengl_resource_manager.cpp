

#include <iostream>

#include "opengl_resource_manager.h"
#include "common_utils.h"
#include "curve_utils.h"


OpenglResourceManager::OpenglResourceManager() {
	std::cout << "Constructor OpenglResourceManager (" << this << ") called " << std::endl;
	m_colors["randomColor"] = CUtils::randomVec3F(0,1);
	m_colors["default"] = glm::vec3(0.0f, 1.0f, 0.0f);
}

OpenglResourceManager::~OpenglResourceManager() {
	std::cout << "Destructor OpenglResourceManager (" << this << ") called " << std::endl;
}

void OpenglResourceManager::initialize() {

	shaderPrograms.emplace("default", ShaderProgram(CUtils::readFile("res/shaders/v_default.glsl"), CUtils::readFile("res/shaders/f_default.glsl")));
	shaderPrograms.emplace("custom", ShaderProgram(CUtils::readFile("res/shaders/v_default.glsl"), CUtils::readFile("res/shaders/f_custom_color.glsl")));

	VBOLayout menuVBOLayout;
	menuVBOLayout.addLayoutElement(2, GL_FLOAT, GL_FALSE);

	const GLfloat vertex[] = {
		//x(s)  y(t)
		0.0f, 1.0f,
		1.0f, -1.0f,
		-1.0f, -1.0f
	};
	VAO baseVAO;
	VBO baseVBO;

	baseVAO.bind();
	baseVBO.init(vertex, 2 * 3 * sizeof(GLfloat));
	baseVAO.addBuffer(baseVBO, menuVBOLayout, 3);
	baseVBO.unbind();
	baseVAO.unbind();

	m_vao.emplace("default", std::move(baseVAO));
	// for test
	auto coords = CurveUtils::createCircleCoords(0, 0, 0.7, 32);
	addRenderObject("circle", coords);
}

void OpenglResourceManager::destroy() {
	shaderPrograms.clear();
	m_colors.clear();
	m_vao.clear();
	m_ebo.clear();
}


ShaderProgram& OpenglResourceManager::getProgram(const std::string& progName)
{
	auto it = shaderPrograms.find(progName);
	if (it != shaderPrograms.end()) {
		return it->second;
	}
	return shaderPrograms.find("default")->second;
}

VAO& OpenglResourceManager::getVAO(const std::string& vaoName)
{
	auto it = m_vao.find(vaoName);
	if (it != m_vao.end()) {
		return it->second;
	}
	return m_vao.find("default")->second;
}

EBO& OpenglResourceManager::getEBO(const std::string& vaoName)
{
	auto it = m_ebo.find(vaoName);
	if (it != m_ebo.end()) {
		return it->second;
	}
	return m_ebo.find("default")->second;
}

glm::vec3& OpenglResourceManager::getColor(const std::string& colorName)
{
	auto it = m_colors.find(colorName);
	if (it != m_colors.end()) {
		return it->second;
	}
	return m_colors.find("default")->second;
}


void OpenglResourceManager::addRenderObject(const std::string& uuid, const std::vector<glm::vec3>& coordsArray)
{
	VBOLayout layout;
	layout.addLayoutElement(2, GL_FLOAT, GL_FALSE);

	auto vertices = std::vector<float>();
	for (auto& x : coordsArray) {
		vertices.push_back(x.x);
		vertices.push_back(x.y);
	}
	// auto vertices = CurveUtils::createLineGeometry(coordsArray, 0.005);
	VAO objVAO;
	VBO objVBO;

	uint64_t count = vertices.size() / 2;
	objVAO.bind();
	objVBO.init(vertices.data(), 2 * count * sizeof(GLfloat));
	objVAO.addBuffer(objVBO, layout, count);
	objVBO.unbind();
	objVAO.unbind();

	m_vao.emplace(uuid, std::move(objVAO));
}
