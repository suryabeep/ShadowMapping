#pragma once

#include "utilities.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Model {
public:
    Model(const char* path) {
        loadObj(path, _vertices, _normals, _texcoords);
    }

    void setupBuffers();
    void draw();
    void deleteGLResources();

private:
    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec2> _texcoords;
    std::vector<glm::vec3> _normals;

    GLuint _vao;
    GLuint _vertexBuffer;
    GLuint _texcoordBuffer;
    GLuint _normalBuffer;
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

    // texcoord buffer
    glGenBuffers(1, &_texcoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _texcoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, _texcoords.size() * sizeof(_texcoords.at(0)), _texcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(glm::vec2), (void *) 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Model::deleteGLResources() {
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteBuffers(1, &_texcoordBuffer);
    glDeleteBuffers(1, &_normalBuffer);
    glDeleteVertexArrays(1, &_vao);
}

void Model::draw() {
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
}