#pragma once

#include "utilities.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Model {
public:
    Model(const char* path) {
        loadObj(path, _vertices, _faces, _normals);

        fprintf(stderr, "Vertices size is %lu\n", _vertices.size());
        fprintf(stderr, "Normals size is %lu\n", _normals.size());
        fprintf(stderr, "Faces size is %lu\n", _faces.size());
    }

    void setupBuffers();
    void draw();
    void deleteGLResources();

private:
    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec3> _normals;
    std::vector<glm::ivec3> _faces;

    GLuint _vao;
    GLuint _vertexBuffer;
    GLuint _normalBuffer;
    GLuint _faceBuffer;
};

void Model::setupBuffers() {
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // vertex buffer
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(_vertices.at(0)), _vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), (void *) 0);
    glEnableVertexAttribArray(0);

    // normal buffer
    glGenBuffers(1, &_normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, _normals.size() * sizeof(_normals.at(0)), _normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(glm::vec3), (void *) 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Model::deleteGLResources() {
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteBuffers(1, &_faceBuffer);
    glDeleteVertexArrays(1, &_vao);
}

void Model::draw() {
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
}