#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 vert_tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 frag_tex_coord;

void main () {
	gl_Position = projection * view * model * vec4(pos, 1.0);
	frag_tex_coord = vert_tex_coord;
}
