// #version 410

// in VS_OUT {
// 	vec3 normal;
//     vec2 tex_coord;
// } fs_in;

// out vec4 frag_color;

// uniform sampler2D nebula;

// void main()
// {
// 	frag_color = texture(nebula, fs_in.tex_coord);
// }

#version 410
 
uniform float ellapsed_time_s;

in VS_OUT {
	vec3 normal;
    vec2 tex_coord;
} fs_in;
 
out vec4 frag_color;
 
uniform sampler2D nebula;
 
void main()
{
	vec2 texSpeed = vec2(0.01, 0);
	float texTime = mod(ellapsed_time_s, 100.0);
	vec2 newTexCoord = vec2(fs_in.tex_coord.y, fs_in.tex_coord.x) + texSpeed*texTime;
	newTexCoord += texSpeed*texTime;
	frag_color = texture(nebula, newTexCoord);
	// frag_color = texture(nebula, fs_in.tex_coord);
}
