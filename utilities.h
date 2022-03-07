#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>
#include <string>
#include <iostream>

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// simple OBJ file loader modified from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
bool loadObj (const char * path, std::vector < glm::vec3 > & out_vertices, std::vector < glm::ivec3 > & out_faces) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Unable to open the file! \n");
        return false;
    }

    // read line by line until EOF
    while (true) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF) {
            break;
        }

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            out_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            glm::ivec3 face;
            fscanf(file, "%d %d %d\n", &face.x, &face.y, &face.z);
            // adjust indexing
            face -= glm::ivec3(1, 1, 1);
            out_faces.push_back(face);
        }
    }
    return true;
}

// math from https://math.stackexchange.com/a/2686620
glm::vec4 computePlaneCoeffs(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    glm::vec3 first_3 = glm::cross(b - a, c - a);
    float k = -glm::dot(first_3, a);
    return glm::vec4(first_3, k);
}

bool loadObj (const char* path, std::vector<glm::vec3> &out_vertices, std::vector<glm::ivec3> &out_faces, std::vector<glm::vec3> &out_normals) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Unable to open the file! \n");
        return false;
    }

    std::cerr << "Made it to line " << __LINE__ << std::endl;

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<glm::vec2> temp_texcoords;

    // read line by line until EOF
    while (true) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF) {
            break;
        }

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_positions.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 tex;
            fscanf(file, "%f %f\n", &tex.x, &tex.y);
            temp_texcoords.push_back(tex);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            glm::ivec3 vertex_indices;
            glm::ivec3 tex_indices;
            glm::ivec3 normal_indices;

            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertex_indices.x, &tex_indices.x, &normal_indices.x,
                                                         &vertex_indices.y, &tex_indices.y, &normal_indices.y, 
                                                         &vertex_indices.z, &tex_indices.z, &normal_indices.z);
            
            if (matches != 9) {
                fprintf(stderr, "Could not read line in obj file!\n");
                return false;
            }

            // adjust indexing  
            vertex_indices -= glm::ivec3(1, 1, 1);
            tex_indices -= glm::ivec3(1, 1, 1);
            normal_indices -= glm::ivec3(1, 1, 1);

            assert(vertex_indices.x < temp_positions.size());
            assert(vertex_indices.y < temp_positions.size());
            assert(vertex_indices.z < temp_positions.size());

            out_vertices.push_back(temp_positions[vertex_indices.x]);
            out_vertices.push_back(temp_positions[vertex_indices.y]);
            out_vertices.push_back(temp_positions[vertex_indices.z]);

            assert(normal_indices.x < temp_normals.size());
            assert(normal_indices.y < temp_normals.size());
            assert(normal_indices.z < temp_normals.size());

            out_normals.push_back(temp_normals[normal_indices.x]);
            out_normals.push_back(temp_normals[normal_indices.y]);
            out_normals.push_back(temp_normals[normal_indices.z]);
        }
        
    }

    std::cerr << "Made it to line " << __LINE__ << std::endl;

    return true;
}
