#version 460 core

layout (location = 0) in vec2 TexCoord;

layout (location = 0) out vec4 fragColor;

uniform sampler2D depthMap;

void main() {
    fragColor = vec4(vec3(texture(depthMap, TexCoord).r), 1);    
}