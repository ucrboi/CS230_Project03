#ifndef PLANET_H
#define PLANET_H

#include <glm/glm.hpp>
#include <vector>

struct Planet
{
    glm::vec2 position;
    float radius;
    glm::vec3 color;
    float mass;
    float orbitRadius;
};

struct Body
{
    float radius;
    float mass;
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec3 color;
};

struct Asteroid
{
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 prevPosition;
};

glm::vec2 computeAcceleration(const glm::vec2 &pos, const std::vector<Planet> &planets);

#endif // PLANET_H
