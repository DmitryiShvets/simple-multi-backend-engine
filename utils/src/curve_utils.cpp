#include "curve_utils.h"
#include <cmath>

std::vector<glm::vec3> CurveUtils::createCircleGeometry(float x, float y, float radius, float segments)
{
    std::vector<glm::vec3> vertices;
    auto angleStep = (PI * 2) / segments;
    for (int i = 0; i < segments; i++) {
        auto angle1 = i * angleStep;
        auto angle2 = (i + 1) * angleStep;
        auto sin1 = std::sin(angle1);
        auto cos1 = std::cos(angle1);
        auto sin2 = std::sin(angle2);
        auto cos2 = std::cos(angle2);
        vertices.emplace_back(glm::vec3(x, y, x + cos1 * radius));
        vertices.emplace_back(glm::vec3(y + sin1 * radius, x + cos2 * radius, y + sin2 * radius));
    }
    return vertices;
}

std::vector<glm::vec3> CurveUtils::createCircleCoords(float x, float y, float radius, float segments)
{
    std::vector<glm::vec3> coords;
    coords.reserve(segments);

    const float segmentAngle = 2.0f * PI / segments;

    for (int i = 0; i <= segments; ++i) {
        float angle = i * segmentAngle;
        float xPos = x + radius * cos(angle);
        float yPos = y + radius * sin(angle);

        coords.emplace_back(xPos, yPos, 0.0f);
    }

    return coords;
}

std::vector<glm::vec3> CurveUtils::getSegmentSubdivision(const glm::vec3& p1, const glm::vec3& p2, float thickness) {
    const float dx = p2.x - p1.x;
    const float dy = p2.y - p1.y;
    const float dz = p2.z - p1.z;
    const float len = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (len < 1e-6f) {
        return std::vector<glm::vec3>(6, glm::vec3(0.0f));
    }

    // Calculate normal vector for thickness offset (in XY plane)
    const float nx = (dy / len) * (thickness / 2.0f);
    const float ny = (-dx / len) * (thickness / 2.0f);

    // 2 triangles per segment (6 vertices)
    // Triangle 1: vertices 0, 1, 2
    // Triangle 2: vertices 3, 4, 5
    return {
        glm::vec3(p1.x + nx, p1.y + ny, p1.z),   // Vertex 0
        glm::vec3(p1.x - nx, p1.y - ny, p1.z),    
        glm::vec3(p2.x + nx, p2.y + ny, p2.z),   

        glm::vec3(p2.x + nx, p2.y + ny, p2.z),   
        glm::vec3(p1.x - nx, p1.y - ny, p1.z),   
        glm::vec3(p2.x - nx, p2.y - ny, p2.z)    // Vertex 5
    };
}

std::vector<float> CurveUtils::createLineGeometry(const std::vector<glm::vec3>& points, float thickness) {
    std::vector<float> vertices;

    for (size_t i = 0; i < points.size() - 1; ++i) {
        const auto& p1 = points[i];
        const auto& p2 = points[i + 1];

        auto segmentVertices = getSegmentSubdivision(p1, p2, thickness);

        for (const auto& vertex : segmentVertices) {
            vertices.push_back(vertex.x);
            vertices.push_back(vertex.y);
        }
    }

    return vertices;
}

std::vector<glm::vec3> CurveUtils::createRectangleCoords(float centerX, float centerY, float sizeX, float sizeY) {
    std::vector<glm::vec3> coords;
    coords.reserve(5);

    const float xHalfSide = sizeX / 2.0f;
    const float yHalfSide = sizeY / 2.0f;

    coords.emplace_back(centerX - xHalfSide, centerY - yHalfSide, 0.0f);
    coords.emplace_back(centerX + xHalfSide, centerY - yHalfSide, 0.0f);
    coords.emplace_back(centerX + xHalfSide, centerY + yHalfSide, 0.0f);
    coords.emplace_back(centerX - xHalfSide, centerY + yHalfSide, 0.0f);
    coords.emplace_back(centerX - xHalfSide, centerY - yHalfSide, 0.0f);

    return coords;
}