#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "assets/Mesh.hpp"
#include "render/ShaderProgram.hpp"
#include "render/Texture.hpp"

class Model {
private:
    // origin point of whole model
    glm::vec3 pivot_position{}; // [0,0,0] of the object
    glm::vec3 euler_angles{};    // pitch, yaw, roll
    glm::vec3 scale{ 1.0f };

    glm::mat4 local_model_matrix{ 1.0 }; //cache, and for complex transformations (default = identity)

    glm::mat4 create_MM(const glm::vec3& origin, const glm::vec3& e_ang, const glm::vec3& scale) {
        // keep angles in proper range
        glm::vec3 eA{ wrap_angle(e_ang.x), wrap_angle(e_ang.y), wrap_angle(e_ang.z) };

        glm::mat4 t = glm::translate(glm::mat4(1.0f), origin);
        glm::mat4 rotm = glm::yawPitchRoll(glm::radians(eA.y), glm::radians(eA.x), glm::radians(eA.z)); //yaw, pitch, roll
        glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);

        return s * rotm * t;
    }

    float wrap_angle(float angle) { // wrap any float to [0, 360)
        angle = std::fmod(angle, 360.0f);
        if (angle < 0.0f) {
            angle += 360.0f;
        }
        return angle;
    }

    void process_node(aiNode* node, const aiScene* scene, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Texture> texture = nullptr) {
        // Process all meshes in node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; // Get mesh from scene
            add_mesh(process_mesh(mesh), shader, texture); // Convert to mesh and add to model
         }
        // Recursive walkthrough the model's tree
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            process_node(node->mChildren[i], scene, shader, texture);
        }
    }
public:
    // mesh related data
    typedef struct mesh_package {
        std::shared_ptr<Mesh> mesh;         // geometry & topology, vertex attributes
        std::shared_ptr<ShaderProgram> shader;     // which shader to use to draw this part of the model
        std::shared_ptr<Texture> texture;     // which texture to use to draw this part of the model

        glm::vec3 origin;                   // mesh origin relative to origin of the whole model
        glm::vec3 euler_angles;              // mesh rotation relative to orientation of the whole model
        glm::vec3 scale;                    // mesh scale relative to scale of the whole model
    } MeshPackage;
    std::vector<MeshPackage> meshes;

    Model() = default;
    Model(const std::filesystem::path& filename, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Texture> texture = nullptr) {
        // Initialize and prepare .obj reader
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filename.string(),
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_JoinIdenticalVertices);

        // Safety check
        if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            throw std::runtime_error("Failed to load model: " + filename.string());
        }

        // Start recursive walkthrough the model's tree
        process_node(scene->mRootNode, scene, shader, texture);
    }

    std::shared_ptr<Mesh> process_mesh(aiMesh* mesh) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v;
            v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            v.normal = mesh->HasNormals() ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : glm::vec3(0.0f);
            v.tex_coords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
            vertices.push_back(v);
        }

        // Process vertex indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Create final mesh
        return std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES);
    }

    void add_mesh(std::shared_ptr<Mesh> mesh,
        std::shared_ptr<ShaderProgram> shader,
        std::shared_ptr<Texture> texture = nullptr, // default no texture
        glm::vec3 origin = glm::vec3(0.0f),      // dafault value
        glm::vec3 euler_angles = glm::vec3(0.0f), // dafault value
        glm::vec3 scale = glm::vec3(1.0f)       // dafault value
        ) {
        meshes.emplace_back(mesh, shader, texture, origin, euler_angles, scale);
    }

    // update based on running time
    void update(const float delta_t) {
        // change internal state of the model (positions of meshes, size, etc.) 
        // note: this allows dynamic behaviour - it can be modified to 
        //       use lambda funtion, call scripting language, etc. 
    }

    void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix) {
        // call draw() on mesh (all meshes)
        for (auto const& mesh_pkg : meshes) {
            mesh_pkg.shader->use(); // select proper shader

            // Set view and projection matrices
            mesh_pkg.shader->set_uniform("uV_m", view_matrix);
            mesh_pkg.shader->set_uniform("uP_m", projection_matrix);

            // Bind the texture
            if (mesh_pkg.texture) {
                mesh_pkg.texture->bind();
            }

            // Calculate and set model matrix
            glm::mat4 mesh_model_matrix = create_MM(mesh_pkg.origin, mesh_pkg.euler_angles, mesh_pkg.scale);
            mesh_pkg.shader->set_uniform("uM_m", mesh_model_matrix * local_model_matrix);

            mesh_pkg.mesh->draw();   // draw mesh
        }
    }

#pragma region Bounding box
    AABB get_local_AABB() const {
        bool first = true;
        AABB result{};

        for (const auto& pkg : meshes) {
            const AABB& meshBox = pkg.mesh->get_local_AABB();

            glm::vec3 min = meshBox.min * pkg.scale + pkg.origin;
            glm::vec3 max = meshBox.max * pkg.scale + pkg.origin;

            if (first) {
                result.min = min;
                result.max = max;
                first = false;
            }
            else {
                result.min = glm::min(result.min, min);
                result.max = glm::max(result.max, max);
            }
        }

        return result;
    }

    AABB get_world_AABB() const {
        AABB local = get_local_AABB();

        glm::vec3 worldMin = local.min * scale + pivot_position;
        glm::vec3 worldMax = local.max * scale + pivot_position;

        return { worldMin, worldMax };
    }

#pragma endregion

#pragma region Transformations
    void set_position(const glm::vec3& new_position) {
        pivot_position = new_position;
        local_model_matrix = create_MM(pivot_position, euler_angles, scale);
    }

    glm::vec3 get_position() {
        return pivot_position;
    }

    void seteuler_angles(const glm::vec3& new_euler_angles) {
        euler_angles = new_euler_angles;
        local_model_matrix = create_MM(pivot_position, euler_angles, scale);
    }

    void set_scale(const glm::vec3& new_scale) {
        scale = new_scale;
        local_model_matrix = create_MM(pivot_position, euler_angles, scale);
    }

    // for complex (externally provided) transformations 
    void set_model_matrix(const glm::mat4& modelm) {
        local_model_matrix = modelm;
    }

    void translate(const glm::vec3& offset) {
        pivot_position += offset;
        local_model_matrix = create_MM(pivot_position, euler_angles, scale);
    }

    void rotate(const glm::vec3& pitch_yaw_roll_offs) {
        euler_angles += pitch_yaw_roll_offs;
        local_model_matrix = create_MM(pivot_position, euler_angles, scale);
    }

    void scale_by(const glm::vec3& scale_offs) {
        scale *= scale_offs;
        local_model_matrix = create_MM(pivot_position, euler_angles, scale);
    }
#pragma endregion

};
