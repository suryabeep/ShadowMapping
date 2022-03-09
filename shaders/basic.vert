#version 460 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 texCoord;

layout (location = 0) out vec3 positionWorldSpace;
layout (location = 1) out vec3 vertexNormalWorldSpace;
layout (location = 2) out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main() {
	positionWorldSpace = vec3(model * vec4(vertexPosition, 1.0));
	vertexNormalWorldSpace = normalize(transpose(inverse(mat3(model))) * vertexPosition);
	fragPosLightSpace = lightSpaceMatrix * vec4(positionWorldSpace, 1.0);
	gl_Position = projection * view * vec4(positionWorldSpace, 1.0f);
}