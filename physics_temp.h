#ifndef PHYSICS_H
#define PHYSICS_H

#include "bodies.h"
#include <vector>
#include <cmath>

// Constants
// const float G = 6.67430e-11f; // Gravitational constant (scaled for 2D simulation)
// const float G = 6.67430e-5f; // Scaled gravitational constant for simulation
// Revised constants (adjusted for 2D orbital mechanics)
// physics.h
const float G = 0.001f;         // Reduced by 100x
const float SUN_MASS = 1000.0f; // Keep scaled-down solar mass

// Forward declaration
glm::vec2 computeAcceleration(const glm::vec2 &pos, const std::vector<Planet> &planets);

void updateAsteroids(std::vector<Asteroid> &asteroids, const std::vector<Planet> &planets, float deltaTime)
{
    for (auto &asteroid : asteroids)
    {
        glm::vec2 acceleration_prev = computeAcceleration(asteroid.position, planets);
        asteroid.velocity += 0.5f * acceleration_prev * deltaTime;
        asteroid.position += asteroid.velocity * deltaTime;
        glm::vec2 acceleration_new = computeAcceleration(asteroid.position, planets);
        asteroid.velocity += 0.5f * acceleration_new * deltaTime;
    }
}

glm::vec2 computeAcceleration(const glm::vec2 &pos, const std::vector<Planet> &planets)
{
    glm::vec2 total(0.0f);
    for (const auto &planet : planets)
    {
        glm::vec2 dir = planet.position - pos;
        float dist = glm::length(dir);
        if (dist > 0.01f)
        {
            dir = glm::normalize(dir);
            total += G * planet.mass * dir / (dist * dist);
        }
    }
    return total;
}

void updatePhysics(std::vector<Planet> &planets, std::vector<Asteroid> &asteroids)
{
    static double previousTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float deltaTime = static_cast<float>(currentTime - previousTime);
    previousTime = currentTime;

    // Update planet positions (if they orbit)
    for (size_t i = 1; i < planets.size(); ++i)
    {
        float orbitRadius = planets[i].orbitRadius;
        float orbitSpeed = 1.0f / orbitRadius;
        planets[i].position.x = orbitRadius * cos(currentTime * orbitSpeed);
        planets[i].position.y = orbitRadius * sin(currentTime * orbitSpeed);
    }

    // Update asteroid positions using FMM
    updateAsteroids(asteroids, planets, deltaTime);
}

#endif // PHYSICS_H
