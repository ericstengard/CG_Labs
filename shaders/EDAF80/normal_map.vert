// #version 410

// layout (location = 0) in vec3 vertex;
// layout (location = 1) in vec3 normal;
// layout (location = 2) in vec3 texcoord;
// layout (location = 3) in vec3 tangent;
// layout (location = 4) in vec3 binormal;

// uniform mat4 normal_model_to_world;


// out VS_OUT {
// 	vec3 vertex;
//     vec2 tex_coord;
// 	vec3 normal;
//     vec3 tangent;
//     vec3 binormal;
//     // mat3 TBN;
// } vs_out;

// void main()
// {
//     vs_out.tex_coord = vec2(texcoord.x, texcoord.y);
// 	vs_out.vertex = vec3(vertex_model_to_world * vec4(vertex, 1.0));

//     vec3 T = normalize(vec3(tangent));
//     vec3 B = normalize(vec3(binormal));
//     vec3 N = normalize(vec3(normal));

//     // vs_out.TBN = mat3(T, B, N);
//     vs_out.tangent = T;
//     vs_out.normal = N;
//     vs_out.binormal = B;

//     gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
// }