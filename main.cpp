#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "utilities.h"
#include "shader.h"
#include "camera.h"
#include "model.h"

GLFWwindow* initWindow();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(4, 2, 4), glm::vec3(0, 1, 0), -77.0f, -16.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Light
glm::vec3 lightPos{-2, 4, 0};
glm::vec3 lightDir{1.0, -1.0, -1.0};

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// wireframe mode
bool wireframe = false;

int main()
{
    GLFWwindow* window = initWindow();

    glEnable(GL_DEPTH_TEST);

    Shader *basicShader = new Shader("shaders/basic.vert", "shaders/basic.frag");
    Shader *depthShader = new Shader("shaders/simpleDepth.vert", "shaders/simpleDepth.frag");

    Model *cube1 = new Model("cube.obj");
    cube1->setupBuffers();
    GLuint brickTexture = loadTexture("resources/brickwall.jpg");

    // set up buffers for the dubgging quad
    Shader* passthroughShader = new Shader("shaders/passthrough.vert", "shaders/passthrough.frag");
    GLuint quadVAO;
    GLuint quadVerticesBuffer;
    GLuint quadTexCoordsBuffer;
    float quadVertices[18] = {
        -1.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        -1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, 0.0, 0.0
    };
    float quadTexCoords[12] {
        0.0, 0.0,
        1.0, 0.0,
        0.0, 1.0,
        0.0, 1.0, 
        1.0, 0.0,
        1.0, 1.0
    };
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
    // vertex buffer
    glGenBuffers(1, &quadVerticesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadVerticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), &quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // texcoord buffer
    glGenBuffers(1, &quadTexCoordsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadTexCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), &quadTexCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 2 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
   

    glm::mat4 modelMat = glm::mat4(1);
    glm::mat4 view;
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    auto trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5, -2.0));
    auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(10, 0.5, 10));


    // Shadow Map stuff
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
   
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // First render to depth map
        // configure shader and matrices
        float near_plane = 1.0f, far_plane = 100.0f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(lightPos, 
                                        glm::vec3(0.0, 0.0, -2.0), 
                                        glm::vec3( 0.0f, 1.0f,  0.0f));
        glm::mat4 lightSpaceMatrix =  lightProjection * lightView;
        depthShader->use();
        depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            glCullFace(GL_FRONT);
            // render scene
            modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -5.0f));
            depthShader->setMat4("model", modelMat);
            cube1->draw();

            // cube2
            modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 2.0f, -3.0f));
            depthShader->setMat4("model", modelMat);
            cube1->draw();

            // floor
            modelMat = trans * scale;
            depthShader->setMat4("model", modelMat);
            cube1->draw();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // Then render the scene as normal with shadow mapping
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.82, 0.93, 0.99, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);


        // Render the debugging quad
        passthroughShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        passthroughShader->setInt("depthMap", 0);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Render the rest of the cubes
        view = camera.GetViewMatrix();
        glm::vec3 camPos = camera.Position;

        basicShader->use();
        basicShader->setVec3("lightPos", lightPos);
        basicShader->setVec3("eyePos", camPos);
        basicShader->setMat4("view", view);
        basicShader->setMat4("projection", projection);
        basicShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        basicShader->setInt("depthMap", 0);

        // cube1
        modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -5.0f));
        basicShader->setMat4("model", modelMat);
        cube1->draw();

        // cube2
        modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 2.0f, -3.0f));
        basicShader->setMat4("model", modelMat);
        cube1->draw();

        // floor
        modelMat = trans * scale;
        basicShader->setMat4("model", modelMat);
        cube1->draw();

        // lightCube
        modelMat = glm::translate(glm::mat4(1.0f), lightPos) * glm::scale(glm::mat4(1.0f), glm::vec3(0.3));
        basicShader->setMat4("model", modelMat);
        cube1->draw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cube1->deleteGLResources();
    delete cube1;
    delete basicShader;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    float increment = 0.05;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        lightPos.y +=increment;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        lightPos.x -=increment;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        lightPos.y -= increment;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        lightPos.x += increment;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        lightPos.z += increment;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        lightPos.z -= increment;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        fprintf(stderr, "light pos = [ %f %f %f]\n", lightPos.x, lightPos.y, lightPos.z);
        fprintf(stderr, "cam pos = [ %f %f %f]\n", camera.Position.x, camera.Position.y, camera.Position.z);
    }
    
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


GLFWwindow* initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    return window;
}