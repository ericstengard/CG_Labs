#version 410

uniform bool use_normal_mapping;
out vec4 frag_color;
uniform sampler2D normal_map;
uniform sampler2D diffuse_map;
uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;
uniform mat4 normal_model_to_world;

in VS_OUT {
	vec3 vertex;
    vec2 tex_coord;
	vec3 normal;
    vec3 tangent;
    vec3 binormal;
} fs_in;


void main()
{
	vec3 L = normalize(light_position - fs_in.vertex);
	vec3 V = normalize(camera_position - fs_in.vertex);
	vec3 diffuse_color;
	vec3 n;

	if(use_normal_mapping)
	{
		mat3 TBN = mat3(normalize(fs_in.tangent),normalize(fs_in.binormal), normalize(fs_in.normal));
		//mat3 TBN = mat3(fs_in.tangent,fs_in.binormal, fs_in.normal);
		n = texture(normal_map, fs_in.tex_coord).xyz*2 - 1;
		vec4 normal_world = normal_model_to_world * vec4(TBN * n, 0.0f);
		n = normalize(normal_world.xyz);
		
		diffuse_color = texture(diffuse_map, fs_in.tex_coord).rgb * max(dot(n, L),0.0f);
	}
	else
	{
		//vs_out.normal = vec3(normal_model_to_world * vec4(normal, 0.0));
		n = normalize(vec3(normal_model_to_world * vec4(fs_in.normal, 0.0)));
		diffuse_color = texture(diffuse_map, fs_in.tex_coord).rgb * max(dot(n, L),0.0f);
	}

	vec3 specular = specular * pow( max(dot(reflect(-L,n),V),0),shininess) ;
	frag_color = vec4(ambient + diffuse_color + specular, 1.0);

}