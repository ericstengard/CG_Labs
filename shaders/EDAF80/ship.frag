#version 410

in VS_OUT {
	vec3 normal;
    vec2 tex_coord;
} fs_in;

out vec4 frag_color;

uniform sampler2D nebula;

void main()
{
	frag_color = texture(nebula, fs_in.tex_coord);
}