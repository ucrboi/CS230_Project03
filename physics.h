#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>
#include <glm/glm.hpp>
#include "bodies.h"

const float G = 1.0;
const float softening = 0.001;

glm::vec2 computeForce(const Body &a, const Body &b)
{
    glm::vec2 direction = b.position - a.position;
    float distance = glm::length(direction);
    float forceMagnitude = (G * a.mass * b.mass) / (distance * distance + softening * softening);
    glm::vec2 force = forceMagnitude * glm::normalize(direction);
    return force;
}

void updatePhysics(std::vector<Body> &bodies, float dt)
{
    // std::vector<glm::vec2> forces(bodies.size(), glm::vec2(0.0f));

    // for (size_t i = 0; i < bodies.size(); i++)
    // {
    //     for (size_t j = 0; j < bodies.size(); j++)
    //     {
    //         glm::vec2 force = computeForce(bodies[i], bodies[j]);
    //         forces[i] += force;
    //         forces[j] -= force;
    //     }
    // }

    // for (size_t i = 0; i < bodies.size(); i++)
    // {
    //     glm::vec2 acceleration = forces[i] / bodies[i].mass;
    //     bodies[i].velocity += acceleration * dt;
    //     bodies[i].position += bodies[i].velocity * dt;
    // }
}

#endif
