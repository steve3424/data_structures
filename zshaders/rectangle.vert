#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 vert_tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 frag_tex_coords;

void main() {
	gl_Position = projection * view * model * vec4(pos.x, pos.y, pos.z, 1.0);
	frag_tex_coords = vert_tex_coords;
}
