#pragma once

#include "assignment1.hpp"
#include "core/helpers.hpp"
#include "core/node.hpp"


class CelestialBody
{
    public: 
        CelestialBody(  bonobo::mesh_data const& shape, 
                        GLuint const* program, 
                        GLuint diffuse_texture_id
                    );

        glm::mat4 render(   std::chrono::microseconds ellapsed_time, 
                            glm::mat4 const& view_projection, 
                            glm::mat4 const& parent_transform = glm::mat4(1.0f)
                        );

        void set_scale(glm::vec3 const& scale = glm::vec3(1.0f ,1.0f, 1.0f));
        void set_spin(SpinConfiguration const& configuration = {glm::radians(0.0f), 0.0f});

    private:
        Node _body;
        glm::vec3 _scale;
        SpinConfiguration _spinConfiguration;
        float _spin_angle = 0;
};