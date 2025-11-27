#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <memory> 

#include <GL/glew.h>
#include <glm/glm.hpp> 

#include "assets/Mesh.hpp"
#include "render/ShaderProgram.hpp"
#include "utils/OBJloader.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

class Model {
private:
    // origin point of whole model
    glm::vec3 pivot_position{}; // [0,0,0] of the object
    glm::vec3 eulerAngles{};    // pitch, yaw, roll
    glm::vec3 scale{ 1.0f };

    glm::mat4 local_model_matrix{ 1.0 }; //cache, and for complex transformations (default = identity)

    glm::mat4 createMM(const glm::vec3& origin, const glm::vec3& eAng, const glm::vec3& scale) {
        // keep angles in proper range
        glm::vec3 eA{ wrapAngle(eAng.x), wrapAngle(eAng.y), wrapAngle(eAng.z) };

        glm::mat4 t = glm::translate(glm::mat4(1.0f), origin);
        glm::mat4 rotm = glm::yawPitchRoll(glm::radians(eA.y), glm::radians(eA.x), glm::radians(eA.z)); //yaw, pitch, roll
        glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);

        return s * rotm * t;
    }

    float wrapAngle(float angle) { // wrap any float to [0, 360)
        angle = std::fmod(angle, 360.0f);
        if (angle < 0.0f) {
            angle += 360.0f;
        }
        return angle;
    }
public:
    // mesh related data
    typedef struct mesh_package {
        std::shared_ptr<Mesh> mesh;         // geometry & topology, vertex attributes
        std::shared_ptr<ShaderProgram> shader;     // which shader to use to draw this part of the model

        glm::vec3 origin;                   // mesh origin relative to origin of the whole model
        glm::vec3 eulerAngles;              // mesh rotation relative to orientation of the whole model
        glm::vec3 scale;                    // mesh scale relative to scale of the whole model
    } MeshPackage;
    std::vector<MeshPackage> meshes;

    Model() = default;
    Model(const std::filesystem::path& filename, std::shared_ptr<ShaderProgram> shader) {
        // Load mesh (all meshes) of the model, (in the future: load material of each mesh, load textures...)
        // notice: you can load multiple meshes and place them to proper positions, 
        //            multiple textures (with reusing) etc. to construct single complicated Model   
        //
        // This can be done by extending OBJ file parser (OBJ can load hierarchical models),
        // or by your own JSON model specification (or keep it simple and set a rule: 1model=1mesh ...) 
        //

        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        loadOBJ(filename, vertices, indices);

        // TODO look into triangles/triangle_strips
        addMesh(std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES), shader);
    }

    void addMesh(std::shared_ptr<Mesh> mesh,
        std::shared_ptr<ShaderProgram> shader,
        glm::vec3 origin = glm::vec3(0.0f),      // dafault value
        glm::vec3 eulerAngles = glm::vec3(0.0f), // dafault value
        glm::vec3 scale = glm::vec3(1.0f)       // dafault value
        ) {
        meshes.emplace_back(mesh, shader, origin, eulerAngles, scale);
    }

    // update based on running time
    void update(const float delta_t) {
        // change internal state of the model (positions of meshes, size, etc.) 
        // note: this allows dynamic behaviour - it can be modified to 
        //       use lambda funtion, call scripting language, etc. 
    }

    void draw() {
        // call draw() on mesh (all meshes)
        for (auto const& mesh_pkg : meshes) {
            mesh_pkg.shader->use(); // select proper shader

            // Calculate and set model matrix
            glm::mat4 mesh_model_matrix = createMM(mesh_pkg.origin, mesh_pkg.eulerAngles, mesh_pkg.scale);
            mesh_pkg.shader->setUniform("uM_m", mesh_model_matrix * local_model_matrix);

            mesh_pkg.mesh->draw();   // draw mesh
        }
    }

#pragma region Transformations
    void setPosition(const glm::vec3& new_position) {
        pivot_position = new_position;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }

    void setEulerAngles(const glm::vec3& new_eulerAngles) {
        eulerAngles = new_eulerAngles;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }

    void setScale(const glm::vec3& new_scale) {
        scale = new_scale;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }

    // for complex (externally provided) transformations 
    void setModelMatrix(const glm::mat4& modelm) {
        local_model_matrix = modelm;
    }

    void translate(const glm::vec3& offset) {
        pivot_position += offset;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }

    void rotate(const glm::vec3& pitch_yaw_roll_offs) {
        eulerAngles += pitch_yaw_roll_offs;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }

    void scaleBy(const glm::vec3& scale_offs) {
        scale *= scale_offs;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }
#pragma endregion

};

