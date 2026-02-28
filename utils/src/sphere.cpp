#include "sphere.h"
#include <cmath>

SphereGeometry getSphere3D(float radius, uint64_t sector_count, uint64_t stack_count) {
    const float PI = std::numbers::pi;
    SphereGeometry sphere;
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, length_inv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sector_step = 2 * PI / sector_count;
    float stack_step = PI / stack_count;
    float sector_angle, stack_angle;

    // Generate vertices
    for (uint64_t i = 0; i <= stack_count; ++i) {
        stack_angle = PI / 2 - i * stack_step;          // starting from pi/2 to -pi/2
        xy = radius * std::cos(stack_angle);            // r * cos(u)
        z = radius * std::sin(stack_angle);             // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // first and last vertices have same position and normal, but different tex coords
        for (uint64_t j = 0; j <= sector_count; ++j) {
            sector_angle = j * sector_step;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * std::cos(sector_angle);             // r * cos(u) * cos(v)
            y = xy * std::sin(sector_angle);             // r * cos(u) * sin(v)
            sphere.positions.push_back({x,y,z});

            // normalized vertex normal (nx, ny, nz)
            nx = x * length_inv;
            ny = y * length_inv;
            nz = z * length_inv;
            sphere.normals.push_back({nx,ny,nz});

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sector_count;
            t = (float)i / stack_count;
            sphere.text_coords.push_back({s,t});
        }
    }

    // generate CCW index list of sphere triangles
    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    int k1, k2;
    for (uint64_t i = 0; i < stack_count; ++i) {
        k1 = i * (sector_count + 1);     // beginning of current stack
        k2 = k1 + sector_count + 1;      // beginning of next stack

        for (uint64_t j = 0; j < sector_count; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                sphere.indices.push_back(k1);
                sphere.indices.push_back(k2);
                sphere.indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if(i != (stack_count - 1))
            {
                sphere.indices.push_back(k1 + 1);
                sphere.indices.push_back(k2);
                sphere.indices.push_back(k2 + 1);
            }

            // store indices for lines
            // vertical lines for all stacks, k1 => k2
            sphere.line_indices.push_back(k1);
            sphere.line_indices.push_back(k2);
            if(i != 0)  // horizontal lines except 1st stack, k1 => k+1
            {
                sphere.line_indices.push_back(k1);
                sphere.line_indices.push_back(k1 + 1);
            }
        }
    }
    return sphere;
};
