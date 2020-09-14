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
    auto scale_matrix = glm::scale(glm::mat4(1.0f), this->_scale);
    std::chrono::duration<float> const ellapsed_time_s = ellapsed_time;

    _spin_angle +=  ellapsed_time_s.count() * _spinConfiguration.speed;
    _orbit_angle += ellapsed_time_s.count() * _orbitConfiguration.speed;

    auto spin_matrix = glm::rotate(glm::mat4(1.0f), _spin_angle, glm::vec3(0.0f ,1.0f, 0.0f));
    auto tilt_matrix = glm::rotate(spin_matrix , _spinConfiguration.inclination, glm::vec3(0.0f ,0.0f, 1.0f));

    auto planet_matrix =  tilt_matrix * scale_matrix;

    auto rotation_matrix = glm::rotate(glm::mat4(1.0f), _orbit_angle, glm::vec3(0.0f ,1.0f, 0.0f));
    auto translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(_orbitConfiguration.radius, 0.0f, 0.0f));

    auto orbit_matrix = rotation_matrix * translation_matrix;

    auto rotation_matrix_zaxis = glm::rotate(glm::mat4(1.0f), _orbitConfiguration.inclination, glm::vec3(0.0f ,0.0f, 1.0f));

    auto tilted_orbit_matrix = rotation_matrix_zaxis * orbit_matrix;

    auto result_matrix = tilted_orbit_matrix * planet_matrix;

    _body.render(view_projection, result_matrix);

    glm::mat4 ring = glm::rotate(result_matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::vec3 ring_scale_vector = glm::vec3(_ring_scale, 1.0f);
    glm::mat4 ring_scale_matrix = glm::scale(glm::mat4(1.0f), ring_scale_vector);

    glm::mat4 ring_res = ring * ring_scale_matrix;

    _ring.render(view_projection, ring_res);
    return parent_transform * result_matrix;
}

void CelestialBody::set_scale(glm::vec3 const& scale)
{
    this->_scale = scale;
}

void CelestialBody::set_spin(SpinConfiguration const& configuration)
{
    this->_spinConfiguration = configuration;
}

void CelestialBody::set_orbit(OrbitConfiguration const& configuration)
{
    this->_orbitConfiguration = configuration;
}


void CelestialBody::set_ring(bonobo::mesh_data const& shape, GLuint const* program, GLuint diffuse_texture_id, glm::vec2 const& scale)
{
    _ring.set_geometry(shape);
    _ring.set_program(program);
    _ring.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
    _ring_scale = scale;
}