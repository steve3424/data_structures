#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 vert_tex_coord;

uniform float x_offset;
uniform float y_offset;
uniform mat4 transform;

out vec2 frag_tex_coord;

void main () {
	gl_Position = transform * vec4(pos, 1.0);
	frag_tex_coord = vec2(vert_tex_coord.x + x_offset, vert_tex_coord.y + y_offset);
}
