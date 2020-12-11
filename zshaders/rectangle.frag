#version 330 core

in vec2 frag_tex_coords;

uniform sampler2D our_texture;

out vec4 frag_color;

void main() {
	frag_color = texture(our_texture, frag_tex_coords);
}
