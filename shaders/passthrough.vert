#version 460 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 texCoord;

layout (location = 0) out vec2 TexCoord;


void main() {
    TexCoord = texCoord;
	gl_Position = vec4(vertexPosition, 1.0f);
}