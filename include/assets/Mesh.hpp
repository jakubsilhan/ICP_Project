#pragma once

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp> 
#include <glm/ext.hpp>

#include "assets/Vertex.hpp"
#include "utils/NonCopyable.hpp"

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    // Check if a point is inside the AABB
    bool contains(const glm::vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
            point.y >= min.y && point.y <= max.y &&
            point.z >= min.z && point.z <= max.z;
    }

    // Center of AABB
    glm::vec3 center() const {
        return (min + max) * 0.5f;
    }

    // Get half-extents of AABB
    glm::vec3 halfExtents() const {
        return (max - min) * 0.5f;
    }
};

class Mesh : private NonCopyable
{
public:
    // force attribute slots in shaders for all meshes, shaders etc.
    static constexpr GLuint attribute_location_position{ 0 };
    static constexpr GLuint attribute_location_normal{ 1 };
    static constexpr GLuint attribute_location_texture_coords{ 2 };

    // No default constructor. RAII - if constructed, it will be correctly initialized
    // and can be rendered. OpenGL resources are guaranteed to be deallocated using destructor. 
    // Double-free errors are prevented by making class non-copyable (therefore 
    // double destruction of the same OpenGL buffer is prevented). 
    Mesh() = delete;

    // Simple mesh from vertices
    Mesh(std::vector<Vertex> const& vertices, GLenum primitive_type) : primitive_type_{ primitive_type }
    {
        // Calculate bounding box
        localAABB_.min = vertices[0].position;
        localAABB_.max = vertices[0].position;

        for (const auto& v : vertices) {
            localAABB_.min = glm::min(localAABB_.min, v.position);
            localAABB_.max = glm::max(localAABB_.max, v.position);
        }

        // OpenGL setup
        glCreateVertexArrays(1, &vao_);

        glVertexArrayAttribFormat(vao_, attribute_location_position, glm::vec3::length(), GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribBinding(vao_, attribute_location_position, 0);
        glEnableVertexArrayAttrib(vao_, attribute_location_position);

        glVertexArrayAttribFormat(vao_, attribute_location_normal, glm::vec3::length(), GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribBinding(vao_, attribute_location_normal, 0);
        glEnableVertexArrayAttrib(vao_, attribute_location_normal);

        glVertexArrayAttribFormat(vao_, attribute_location_texture_coords, glm::vec2::length(), GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coords));
        glVertexArrayAttribBinding(vao_, attribute_location_texture_coords, 0);
        glEnableVertexArrayAttrib(vao_, attribute_location_texture_coords);

        glCreateBuffers(1, &vbo_);
        GLsizeiptr vbo_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex));
        glNamedBufferData(vbo_, vbo_size, vertices.data(), GL_STATIC_DRAW);

        glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(Vertex));

        // store vertex count 
        count_ = static_cast<GLsizei>(vertices.size());
    }

    // Mesh with indirect vertex addressing. Needs compiled shader for attributes setup. 
    Mesh(std::vector<Vertex> const& vertices, std::vector<GLuint> const& indices, GLenum primitive_type) :
        Mesh{ vertices, primitive_type }
    {
        glCreateBuffers(1, &ebo_);
        GLsizeiptr ebo_size = static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint));
        glNamedBufferData(ebo_, ebo_size, indices.data(), GL_STATIC_DRAW);

        glVertexArrayElementBuffer(vao_, ebo_);

        // store indices count 
        count_ = static_cast<GLsizei>(indices.size());
    }

    void draw() {
        glBindVertexArray(vao_);

        if (ebo_ == 0) {
            glDrawArrays(primitive_type_, 0, count_);
        }
        else {
            glDrawElements(primitive_type_, count_, GL_UNSIGNED_INT, nullptr);
        }
    }

    const AABB& get_local_AABB() const { return localAABB_; }

    ~Mesh() {
        glDeleteBuffers(1, &ebo_);
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    };
private:
    //safe defaults
    GLenum primitive_type_{ GL_POINTS };
    GLsizei count_{ 0 };

    // OpenGL buffer IDs
    // ID = 0 is reserved (i.e. uninitalized)
    GLuint vao_{ 0 };
    GLuint vbo_{ 0 };
    GLuint ebo_{ 0 };

    // Bounding box
    AABB localAABB_;
};
