#version 330 core

layout (location = 0) in vec3 pos;

uniform float x_offset;
uniform float y_offset;
uniform float z_offset;

void main() {
	gl_Position = vec4(pos.x + x_offset, pos.y + y_offset, pos.z + z_offset, 1.0);
}
