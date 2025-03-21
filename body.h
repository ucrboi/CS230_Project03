#ifndef BODY_H
#define BODY_H

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>

extern const float X_MEAN;
extern const float X_STD;
extern const float Y_MEAN;
extern const float Y_STD;
extern const float MASS_SUN;

struct GPUBody
{
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 acceleration;
    float mass;
    float radius;
    float pad[2];
};

class Body
{
public:
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 acceleration;
    glm::vec3 color;
    float mass;
    float radius;

    Body(const glm::vec2 &position,
         const glm::vec2 &velocity,
         const glm::vec3 &color,
         float mass_value,
         float radius_value)
        : position(position),
          velocity(velocity),
          color(color),
          acceleration(0.0f, 0.0f),
          mass(mass_value),
          radius(radius_value)
    {
    }

    void update(float dt)
    {
        velocity += acceleration * dt;
        position += velocity * dt;
        acceleration = glm::vec2(0.0f);
    }
};

void initializeBodies(std::vector<Body> &bodies, int n)
{
    Body sun(
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        MASS_SUN,
        0.2f);

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

    for (int i = 0; i < n; i++)
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

        Body b(
            position,
            velocity,
            glm::vec3(1.0f, 1.0f, 1.0f),
            mass,
            radius);

        bodies.push_back(b);
    }

    // Sort the bodies by position to optimize calculations
    std::sort(bodies.begin(), bodies.end(),
              [](const Body &a, const Body &b)
              {
                  return glm::length2(a.position) < glm::length2(b.position);
              });
}

Body merge_bodies(const Body &b1, const Body &b2)
{

    float new_mass = b1.mass + b2.mass;
    glm::vec2 new_position = (b1.position * b1.mass + b2.position * b2.mass) / new_mass;
    glm::vec2 new_velocity = (b1.velocity * b1.mass + b2.velocity * b2.mass) / new_mass;
    float new_radius = std::cbrt(std::pow(b1.radius, 3) + std::pow(b2.radius, 3));
    glm::vec3 new_color = b1.mass > b2.mass ? b1.color : b2.color;

    return Body(new_position, new_velocity, new_color, new_mass, new_radius);
}

#endif