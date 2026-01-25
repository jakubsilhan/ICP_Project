#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "assets/Vertex.hpp"
#include "assets/Mesh.hpp"
#include "utils/Atlas.hpp"

/*
std::shared_ptr<Mesh> generate_cube() {
    std::vector<Vertex> V{
        {{1, 1, 0}}, // [00]
        {{0, 1, 0}}, // [01]
        {{1, 1, 1}}, // [02]
        {{0, 1, 1}}, // [03]
        {{1, 0, 0}}, // [04]
        {{0, 0, 0}}, // [05]
        {{0, 0, 1}}, // [06]
        {{1, 0, 1}}, // [07]
    };

    std::vector<GLuint> I{ 0, 1, 4, 5, 6, 1, 3, 0, 2, 4, 7, 6, 2, 3 };
    return std::make_shared<Mesh>(V, I, GL_TRIANGLE_STRIP);
}*/

inline Atlas cube_atlas_single{
    {
        {0.0f,0.0f}, // 0 back
        {0.0f,0.0f}, // 1 front
        {0.0f,0.0f}, // 2 left
        {0.0f,0.0f}, // 3 right
        {0.0f,0.0f}, // 4 bottom
        {0.0f,0.0f}, // 5 top
    },
    {1.0f,1.0f}
};

inline Atlas cube_atlas_cross{
    {
        {0.0f,2.0f}, // 0 back
        {2.0f,2.0f}, // 1 front
        {1.0f,2.0f}, // 2 left
        {3.0f,2.0f}, // 3 right
        {2.0f,1.0f}, // 4 bottom
        {2.0f,3.0f}, // 5 top
    },
    {4.0f,4.0f}
};

std::shared_ptr<Mesh> generate_cube(Atlas& atlas = cube_atlas_cross) {

    constexpr float u = 1.0f;
    constexpr float h = u / 2;

    std::vector<glm::vec3> v{
        {-h, -h, -h}, // 0 left bottom back
        { h, -h, -h}, // 1 right bottom back
        { h,  h, -h}, // 2 right top back
        {-h,  h, -h}, // 3 left top back
        {-h, -h,  h}, // 4 left bottom front
        { h, -h,  h}, // 5 right bottom front
        { h,  h,  h}, // 6 right top front
        {-h,  h,  h}, // 7 left top front
    };

    const std::vector<glm::vec3> normals{
        { 0,  0, -u}, // 0 back
        { 0,  0,  u}, // 1 front
        {-u,  0,  0}, // 2 left
        { u,  0,  0}, // 3 right
        { 0, -u,  0}, // 4 bottom
        { 0,  u,  0}, // 5 top
    };

    const std::vector<glm::vec2> tex_coords_plane{
        {0.0f, 0.0f}, // left bottom
        {1.0f, 0.0f}, // right bottom
        {1.0f, 1.0f}, // right top
        {0.0f, 1.0f}, // left top
    };

    const std::vector<Vertex> V{

        // 0 back
        {v[0], normals[0], atlas.get_tile_coords(0, tex_coords_plane[1])}, // 0
        {v[1], normals[0], atlas.get_tile_coords(0, tex_coords_plane[0])}, // 1
        {v[2], normals[0], atlas.get_tile_coords(0, tex_coords_plane[3])}, // 2
        {v[3], normals[0], atlas.get_tile_coords(0, tex_coords_plane[2])}, // 3

        // 1 front
        {v[4], normals[1], atlas.get_tile_coords(1, tex_coords_plane[0])}, // 4
        {v[5], normals[1], atlas.get_tile_coords(1, tex_coords_plane[1])}, // 5
        {v[6], normals[1], atlas.get_tile_coords(1, tex_coords_plane[2])}, // 6
        {v[7], normals[1], atlas.get_tile_coords(1, tex_coords_plane[3])}, // 7

        // 2 left
        {v[0], normals[2], atlas.get_tile_coords(2, tex_coords_plane[0])}, // 8
        {v[4], normals[2], atlas.get_tile_coords(2, tex_coords_plane[1])}, // 9
        {v[7], normals[2], atlas.get_tile_coords(2, tex_coords_plane[2])}, // 10
        {v[3], normals[2], atlas.get_tile_coords(2, tex_coords_plane[3])}, // 11

        // 3 right
        {v[1], normals[3], atlas.get_tile_coords(3, tex_coords_plane[1])}, // 12
        {v[5], normals[3], atlas.get_tile_coords(3, tex_coords_plane[0])}, // 13
        {v[6], normals[3], atlas.get_tile_coords(3, tex_coords_plane[3])}, // 14
        {v[2], normals[3], atlas.get_tile_coords(3, tex_coords_plane[2])}, // 15

        // 4 bottom
        {v[0], normals[4], atlas.get_tile_coords(4, tex_coords_plane[0])}, // 16
        {v[4], normals[4], atlas.get_tile_coords(4, tex_coords_plane[3])}, // 17
        {v[5], normals[4], atlas.get_tile_coords(4, tex_coords_plane[2])}, // 18
        {v[1], normals[4], atlas.get_tile_coords(4, tex_coords_plane[1])}, // 19

        // 5 top
        {v[3], normals[5], atlas.get_tile_coords(5, tex_coords_plane[3])}, // 20
        {v[7], normals[5], atlas.get_tile_coords(5, tex_coords_plane[0])}, // 21
        {v[6], normals[5], atlas.get_tile_coords(5, tex_coords_plane[1])}, // 22
        {v[2], normals[5], atlas.get_tile_coords(5, tex_coords_plane[2])}, // 23

    };

    const std::vector<GLuint> I{
         1, 0, 3,   1, 3, 2, // 0 back
         4, 5, 6,   4, 6, 7, // 1 front
         8, 9,10,   8,10,11, // 2 left
        13,12,15,  13,15,14, // 3 right
        16,19,18,  16,18,17, // 4 bottom
        21,22,23,  21,23,20, // 5 top
    };

    return std::make_shared<Mesh>(V, I, GL_TRIANGLES);

}

std::shared_ptr<Mesh> generate_sphere(unsigned int sectors, unsigned int rings) {
    std::vector<Vertex> V{};
    std::vector<GLuint> I{};

    // Ensure the number of sectors wraps around nicely
    unsigned int totalSectors = sectors + 1;

    // Generate vertices
    for (unsigned int r = 0; r <= rings; ++r) {
        float const R = (float)r / (float)rings;
        // Phi goes from -pi/2 (south pole) to pi/2 (north pole)
        float const phi = -glm::pi<float>() / 2.0f + glm::pi<float>() * R;
        float const y = std::sin(phi);
        float const xz_radius = std::cos(phi);

        for (unsigned int s = 0; s <= sectors; ++s) {
            float const S = (float)s / (float)sectors;
            // Theta goes from 0 to 2*pi
            float const theta = 2.0f * glm::pi<float>() * S;
            float const x = std::cos(theta) * xz_radius;
            float const z = std::sin(theta) * xz_radius;

            glm::vec3 position(x, y, z);
            glm::vec3 normal = glm::normalize(position);
            glm::vec2 texCoords(-S, R);

            V.push_back({
                position,
                normal,
                texCoords
            });
        }
    }

    // Generate indices for a single long triangle strip using degenerate triangles
    for (unsigned int r = 0; r < rings; ++r) {
        // Stitch the current ring to the next one using degenerate triangles
        if (r > 0) {
            // Duplicate the last vertex of the previous strip to start a degenerate triangle
            I.push_back(r * totalSectors + sectors);
            // Duplicate the first vertex of the new strip to form a second degenerate triangle
            I.push_back(r * totalSectors);
        }

        // Generate indices for one horizontal strip (alternating winding isn't needed here if vertices are ordered correctly)
        for (unsigned int s = 0; s <= sectors; ++s) {
            // Top vertex of the quad
            I.push_back(r * totalSectors + s);
            // Bottom vertex of the quad
            I.push_back((r + 1) * totalSectors + s);
        }
    }

    return std::make_shared<Mesh>(V, I, GL_TRIANGLE_STRIP);
}