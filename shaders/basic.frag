#version 460 core

layout (location = 0) in vec3 vertexPositionWorldSpace;
layout (location = 1) in vec3 vertexNormalWorldSpace;
layout (location = 2) in vec4 fragPosLightSpace;

layout (location = 0) out vec4 FragColor;

uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 eyePos;

float shadowCalculation() {
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 lightVector = normalize(lightPos - vertexPositionWorldSpace);
    float bias = max(0.05 * (1.0 - dot(vertexNormalWorldSpace, lightVector)), 0.005);
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    if (projCoords.z > 1.0) {
        shadow = 0.0;
    }
    return shadow;
}

void main() {

    float shininess = 64.0;
    vec3 ambient = vec3(0.82, 0.93, 0.99);
    vec3 diffuse = vec3(0.2, 0.5, 0.8);
    vec3 specular = vec3(1, 1, 1);

    // ambient
    float ambientWeight = 0.15;
    ambient *= ambientWeight;

    // diffuse
    vec3 lightVector = normalize(lightPos - vertexPositionWorldSpace);
    float diffuseWeight = max(dot(lightVector, vertexNormalWorldSpace), 0.0);
    diffuse *= diffuseWeight;

    // specular
    vec3 viewDir = normalize(eyePos - vertexPositionWorldSpace);
    vec3 halfwayDir = normalize(lightVector + viewDir);
    float specularWeight = pow(max(dot(vertexNormalWorldSpace, halfwayDir), 0.0), shininess);
    specular *= specularWeight;

    // calculate shadow
    float shadow = shadowCalculation();
 
    FragColor = vec4((ambient + (1.0 - shadow) * diffuse + (1.0 - shadow) * specular), 1.0);
}
