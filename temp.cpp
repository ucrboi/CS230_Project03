#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <random>
#include <string>
#include <iostream>
#include <cmath>

// Add a velocity vector to our Planet struct
struct Planet
{
    std::string name;
    float radius;
    float orbitRadius;
    float mass;
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec3 color;
};

const GLuint WIDTH = 1024, HEIGHT = 768;
const float G = 0.1f;

float timeAcceleration = 1.0f;      // 1x real-time
float fixedTimeStep = 1.0f / 60.0f; // 60 physics updates/sec
float timeAccumulator = 0.0f;

float SCALE_BY = 12;
float AU_MERCURY_ORBIT = 0.39f;
float AU_VENUS_ORBIT = 0.72f;
float AU_EARTH_ORBIT = 1.0f;
float AU_MARS_ORBIT = 1.52f;
float AU_JUPITER_ORBIT = 5.20f;
float AU_SATURN_ORBIT = 9.58f;
float AU_URANUS_ORBIT = 19.22f;
float AU_NEPTUNE_ORBIT = 30.05f;

std::vector<Planet> planets;

// Error callback
void error_callback(int error, const char *description)
{
    std::cerr << "Error: " << description << std::endl;
}

// Draw a circle using GL_TRIANGLE_FAN
void drawCircle(float cx, float cy, float r, int num_segments)
{
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= num_segments; i++)
    {
        float theta = 2.0f * M_PI * float(i) / float(num_segments);
        float x = r * cos(theta);
        float y = r * sin(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

// Render each planet
void render()
{
    for (const auto &planet : planets)
    {
        glColor3f(planet.color.r, planet.color.g, planet.color.b);
        drawCircle(planet.position.x, planet.position.y, planet.radius, 100);
    }
}

// Initialize the solar system and set initial velocities for circular orbits.
// Here we assume the Sun is at (0,0) with zero velocity. For the planets,
// we use v = sqrt(G * M_sun / r) (perpendicular to the radius) as an initial guess.
void initializeSolarSystem()
{
    // Gravitational constant for our simulation units.
    // Adjust this value to control orbital speeds.

    // Sun (massive and static initially)
    planets.push_back({
        "Sun",
        0.2f,
        0.0f,
        100000.0f,
        glm::vec2(0.0f, 0.0f), // position
        glm::vec2(0.0f, 0.0f), // velocity
        glm::vec3(1.0f, 1.0f, 0.0f),
    });

    // For each planet, place them at (orbit,0) and give an initial velocity (0, v)
    auto addPlanet = [&](const std::string &name, float radius, float orbit, float mass, const glm::vec3 &color)
    {
        float v = sqrt(G * planets[0].mass / orbit); // circular orbit speed around the Sun
        planets.push_back({
            name,
            radius,
            orbit,
            mass,
            glm::vec2(orbit, 0.0f), // initial position along the x-axis
            glm::vec2(0.0f, v),     // initial velocity: perpendicular (for counterclockwise orbit)
            color,
        });
    };

    addPlanet("Mercury", 0.03f, AU_MERCURY_ORBIT, 0.055f, glm::vec3(0.7f, 0.7f, 0.7f));
    addPlanet("Venus", 0.05f, AU_VENUS_ORBIT, 0.815f, glm::vec3(1.0f, 0.7f, 0.0f));
    addPlanet("Earth", 0.06f, AU_EARTH_ORBIT, 1.0f, glm::vec3(0.0f, 0.5f, 1.0f));
    addPlanet("Mars", 0.04f, AU_MARS_ORBIT, 0.107f, glm::vec3(1.0f, 0.3f, 0.0f));
    addPlanet("Jupiter", 0.12f, AU_JUPITER_ORBIT, 317.8f, glm::vec3(0.9f, 0.6f, 0.0f));
    addPlanet("Saturn", 0.10f, AU_SATURN_ORBIT, 95.2f, glm::vec3(0.9f, 0.7f, 0.3f));
    addPlanet("Uranus", 0.08f, AU_URANUS_ORBIT, 14.5f, glm::vec3(0.5f, 0.7f, 1.0f));
    addPlanet("Neptune", 0.07f, AU_NEPTUNE_ORBIT, 17.1f, glm::vec3(0.3f, 0.3f, 1.0f));
}

// Updated updatePhysics function that computes gravitational interactions
// between every pair of bodies. (A softening factor is used to avoid singularities.)
void updatePhysics(std::vector<Planet> &planets, float dt)
{
    const float epsilon = 0.01f;

    // Prepare a force accumulator for each planet.
    std::vector<glm::vec2> forces(planets.size(), glm::vec2(0.0f, 0.0f));

    // PROPER PAIRWISE FORCE CALCULATION (prevents double-counting)
    for (size_t i = 0; i < planets.size(); i++)
    {
        for (size_t j = i + 1; j < planets.size(); j++)
        { // Avoid duplicate pairs
            glm::vec2 delta = planets[j].position - planets[i].position;
            float distance = glm::length(delta);
            if (distance > 0.0f)
            {
                float softening = distance * distance + epsilon * epsilon;
                float forceMag = G * planets[i].mass * planets[j].mass / softening;
                glm::vec2 force = forceMag * delta / distance; // Correct direction

                // Apply equal and opposite forces
                forces[i] += force;
                forces[j] -= force;
            }
        }
    }

    // SEMI-IMPLICIT EULER INTEGRATION
    for (size_t i = 0; i < planets.size(); i++)
    {
        if (i == 0)
            continue; // Keep sun stationary (optional)

        glm::vec2 acceleration = forces[i] / planets[i].mass;
        planets[i].velocity += acceleration * dt;
        planets[i].position += planets[i].velocity * dt;
        // glm::vec2 oldPosition = planets[i].position;
        // planets[i].position += planets[i].velocity * dt + 0.5f * acceleration * dt * dt;
        // planets[i].velocity += 0.5f * (acceleration + newAcceleration) * dt;
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

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Solar System", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    initializeSolarSystem();

    // Camera parameters: initial center and zoom (scale)
    float camX = 0.0f, camY = 0.0f;
    float camScale = 2.0f; // half-width/height of the view

    // Set clear color (black)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float frameTime = static_cast<float>(currentTime - lastTime) * timeAcceleration;
        lastTime = currentTime;

        // Handle time accumulation
        timeAccumulator += frameTime;
        while (timeAccumulator >= fixedTimeStep)
        {
            updatePhysics(planets, fixedTimeStep);
            timeAccumulator -= fixedTimeStep;
        }

        // Process keyboard input for interactivity
        float panSpeed = camScale * 0.01f; // Pan speed is proportional to the current zoom
        float zoomSpeed = 0.2f;            // Fixed zoom speed

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camX -= panSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camX += panSpeed;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camY += panSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camY -= panSpeed;

        if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
            camScale -= zoomSpeed;
        if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
            camScale += zoomSpeed;

        // Reset view when "0" is pressed
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        {
            camX = 0.0f;
            camY = 0.0f;
            camScale = 10.0f;
        }
        if (camScale < 0.1f)
            camScale = 0.1f;

        glClear(GL_COLOR_BUFFER_BIT);

        // Update the projection matrix based on camera parameters
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(camX - camScale, camX + camScale, camY - camScale, camY + camScale, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwSwapInterval(1);
    glfwTerminate();
    return 0;
}
