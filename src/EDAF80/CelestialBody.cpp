#include "CelestialBody.hpp"
#include "core/helpers.hpp"

CelestialBody::CelestialBody(  bonobo::mesh_data const& shape, 
                GLuint const* program, 
                GLuint diffuse_texture_id
            )
{
    _body.set_geometry(shape);
    _body.set_program(program);
    _body.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
}

glm::mat4 CelestialBody::render(  std::chrono::microseconds ellapsed_time, 
                        glm::mat4 const& view_projection, 
                        glm::mat4 const& parent_transform)
{
    auto matrix = glm::scale(glm::mat4(1.0f), this->_scale);
    std::chrono::duration<float> const ellapsed_time_s = ellapsed_time;
    _spin_angle = _spin_angle + ellapsed_time_s.count() * _spinConfiguration.speed;

    auto spin_matrix = glm::rotate(glm::mat4(1.0f), _spin_angle, glm::vec3(0.0f ,1.0f, 0.0f));
    auto tilt_matrix = glm::rotate(spin_matrix , _spinConfiguration.inclination, glm::vec3(0.0f ,0.0f, 1.0f));

    auto result_matrix = tilt_matrix * matrix;

    _body.render(view_projection, result_matrix);
    return result_matrix;
}

void CelestialBody::set_scale(glm::vec3 const& scale)
{
    this->_scale = scale;
}

void CelestialBody::set_spin(SpinConfiguration const& configuration)
{
    this->_spinConfiguration = configuration;
}
