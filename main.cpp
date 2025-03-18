#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <random>
#include <cmath>
#include <iostream>
#include "bodies.h"
#include "physics.h"
#include "utils.h"

const GLuint WIDTH = 1024, HEIGHT = 768;
const float INITIAL_CAM_SCALE = 10.0;

const float MASS_SUN = 10000.0;

const float X_MEAN = 15.0, X_STD = 12.0;
const float Y_MEAN = 0.0, Y_STD = 20.0;

const int NUM_BODIES = 10000;
std::vector<Body> bodies;

void initializeBodies()
{
    Body sun = {
        0.2f,
        MASS_SUN,
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
    };

    bodies.push_back(sun);

    std::random_device rd;
    std::mt19937 gen(rd());

    // Bimodal distribution
    // Either on right of sun or left of sun
    std::normal_distribution<float> x_distribution_1(X_MEAN, X_STD);
    std::normal_distribution<float> x_distribution_2(-X_MEAN, X_STD);
    std::bernoulli_distribution mix_x(0.5);

    std::normal_distribution<float> y_distribution(Y_MEAN, Y_STD);

    std::uniform_real_distribution<float> radius_distribution(0.005f, 0.02f);
    std::uniform_real_distribution<float> density_distribution(0.8f, 2.5f);

    std::uniform_real_distribution<float> speed_multiplier(0.95f, 1.05f);

    for (int i = 0; i < NUM_BODIES; i++)
    {
        float radius = radius_distribution(gen);
        float mass = (density_distribution(gen)) * (radius * radius);

        float x = mix_x(gen) ? x_distribution_1(gen) : x_distribution_2(gen);
        float y = y_distribution(gen);
        glm::vec2 position = {x, y};

        glm::vec2 velocity = {0.0f, 0.0f};
        float distance = std::sqrt(x * x + y * y);
        if (distance > 0.001)
        {
            glm::vec2 perpendicular = glm::normalize(glm::vec2(-y, x));
            float orbitalSpeed = std::sqrt(MASS_SUN / distance);
            orbitalSpeed *= speed_multiplier(gen);
            velocity = orbitalSpeed * perpendicular;
        }

        Body b = {
            radius,
            mass,
            position,
            velocity,
            {1.0f, 1.0f, 1.0f},
        };
        bodies.push_back(b);
    }
}

void render()
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
    initializeBodies();

    float camX = 0.0f, camY = 0.0f;
    float camScale = INITIAL_CAM_SCALE;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        controls(window, &camScale, &camX, &camY, INITIAL_CAM_SCALE);
        render();
        updatePhysics(bodies, 0.01);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}