#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoords;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;

out VS_OUT {
	vec3 normal;
	vec2 tex_coord;
} vs_out;


void main()
{
	// vs_out.normal = normal;
	vs_out.tex_coord = vec2(texcoords.x, texcoords.z);
    vs_out.normal = vec3(normal_model_to_world * vec4(normal, 0.0)); 

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}



