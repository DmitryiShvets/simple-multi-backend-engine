#pragma once
#include <string>
#include <map>

#include "opengl_shader_program.h"
#include "opengl_buffer_objects.h"

class OpenglResourceManager {
public:
    OpenglResourceManager();
    ~OpenglResourceManager();
    OpenglResourceManager(const OpenglResourceManager&) = delete;
    OpenglResourceManager& operator=(const OpenglResourceManager&) = delete;
    OpenglResourceManager& operator=(OpenglResourceManager&& program) = default;
    OpenglResourceManager(OpenglResourceManager&& program) = default;
    void initialize();

    void destroy();

    ShaderProgram& getProgram(const std::string& progName);
    VAO& getVAO(const std::string& vaoName);
    EBO& getEBO(const std::string& vaoName);
    glm::vec3& getColor(const std::string& colorName);
    void addRenderObject(const std::string& uuid, const std::vector<glm::vec3>& coordsArray);

private:
 
    std::map<std::string, ShaderProgram> shaderPrograms;
    std::map<std::string, VAO> m_vao;
    std::map<std::string, EBO> m_ebo;
    std::map<std::string, glm::vec3> m_colors;
};