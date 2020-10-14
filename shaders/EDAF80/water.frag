#version 410

uniform samplerCube skybox;
uniform sampler2D water;
uniform vec3 deep_color;
uniform vec3 shallow_color;
uniform vec3 camera_position;
uniform float time;
uniform mat4 normal_model_to_world;

in VS_OUT {
	vec3 vertex;
	vec3 normal_water;
	vec3 tangent_water;
	vec3 binormal_water;
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{
	mat3 TBN_water = mat3(normalize(fs_in.tangent_water), normalize(fs_in.binormal_water), normalize(fs_in.normal_water));
	vec2 texScale = vec2(8, 4);
	float normalTime = mod(time, 100.0);
	vec2 normalSpeed = vec2(-0.05, 0);
	vec3 normal = vec3(0.0);

	vec3 V = normalize(camera_position - fs_in.vertex);

	vec2 normalCoord0 = (fs_in.texcoord.xy*texScale + normalTime*normalSpeed).xy;
	vec2 normalCoord1 = (fs_in.texcoord.xy*texScale*2 + normalTime*normalSpeed*4).xy;
	vec2 normalCoord2 = (fs_in.texcoord.xy*texScale*4 + normalTime*normalSpeed*8).xy;
	vec3 n1 = (texture(water, normalCoord0).xyz * 2 - 1);
	vec3 n2 = (texture(water, normalCoord1).xyz * 2 - 1);
	vec3 n3 = (texture(water, normalCoord2).xyz * 2 - 1);
	vec3 n_bump = normalize(n1 + n2 + n3);

	normal = (normal_model_to_world * vec4(TBN_water * n_bump, 0)).xyz;
	normal = normalize(normal);

	if (gl_FrontFacing){
		normal = -normal;
	}
	vec3 R = texture(skybox, reflect(-V, normal)).xyz;

	float facing = 1 - max(dot(V, normalize(normal)),0.0);
	vec3 water_color = mix(deep_color, shallow_color, facing);

	float r0 = 0.02037;
	float n_one = 1.33;
	float n_two = 1.0/1.33;
	vec3 refraction;
	if (gl_FrontFacing){
			refraction  = texture(skybox, refract(-V,normal, n_one)).xyz;
	} else {
			refraction  = texture(skybox, refract(-V,normal, n_two)).xyz;
	}
	float fastFresnel = r0 + (1-r0)*pow(1-dot(V,normal),5);

	frag_color = vec4(water_color + R * fastFresnel + refraction * (1 - fastFresnel) ,1.0);
}
