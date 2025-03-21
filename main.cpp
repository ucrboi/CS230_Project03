#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <random>
#include <iostream>

#include "body.h"
#include "simulation.h"
#include "utils.h"

const GLuint WIDTH = 1024, HEIGHT = 768;
const float INITIAL_CAM_SCALE = 50.0;

const int NUM_BODIES = 100000;
const float DT = 0.01;
const bool COLLISION = false;

const float X_MEAN = NUM_BODIES <= 25000 ? 10.0 : 15.0;
const float X_STD = NUM_BODIES <= 25000 ? 3.0 : 10.0;
const float Y_MEAN = 0.0;
const float Y_STD = NUM_BODIES <= 25000 ? 5.0 : 10.0;
const float MASS_SUN = 10000.0;

void render(const std::vector<Body> &bodies)
{
    for (const auto &body : bodies)
    {
        glColor3f(body.color.r, body.color.g, body.color.b);
        drawCircle(body.position.x, body.position.y, body.radius, 100);
    }
}

int main()
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Solar System Evolution", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return -1;
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    float camX = 0.0f, camY = 0.0f;
    float camScale = INITIAL_CAM_SCALE;
    int timeFactor = 1;
    bool shouldMove = false;

    std::vector<Body> bodies;
    initializeBodies(bodies, NUM_BODIES);
    bodies.reserve(NUM_BODIES + 1);

    Simulation sim(NUM_BODIES, DT, bodies);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        controls(window, &camScale, &camX, &camY, INITIAL_CAM_SCALE, &shouldMove);

        if (shouldMove)
            sim.step();
        render(bodies);

        // std::cout << bodies.size() << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}