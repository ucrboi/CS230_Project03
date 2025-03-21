#ifndef QUADTREE_H
#define QUADTREE_H

#include <glm/glm.hpp>
#include <vector>
#include <limits>
#include <cmath>
#include "body.h"
#include <iostream>

class Quad
{
public:
    glm::vec2 center;
    float size;

    Quad(const glm::vec2 &c, float s) : center(c), size(s) {}

    int find_quadrant(const glm::vec2 &pos) const
    {
        return ((pos.y > center.y) << 1) | (pos.x > center.x);
    }

    Quad into_quadrant(int quadrant) const
    {
        Quad new_quad = *this;
        new_quad.size *= 0.5f;

        // Simplified offset calculation
        const float quadrant_x = static_cast<float>((quadrant & 1) * 2 - 1);
        const float quadrant_y = static_cast<float>((quadrant >> 1) * 2 - 1);

        new_quad.center += glm::vec2(
            quadrant_x * new_quad.size * 0.5f,
            quadrant_y * new_quad.size * 0.5f);
        return new_quad;
    }

    std::vector<Quad> subdivide() const
    {
        std::vector<Quad> quads;
        quads.reserve(4);

        for (int i = 0; i < 4; ++i)
        {
            quads.emplace_back(into_quadrant(i));
        }
        return quads;
    }
};

Quad new_quadtree(const std::vector<Body> &bodies)
{
    if (bodies.empty())
        return Quad(glm::vec2(0), 0.0f);

    // Initialize with first body's position
    glm::vec2 min = bodies[0].position;
    glm::vec2 max = min;

    for (const auto &body : bodies)
    {
        min = glm::min(min, body.position);
        max = glm::max(max, body.position);
    }

    const glm::vec2 center = (min + max) * 0.5f;
    const float size = glm::max(max.x - min.x, max.y - min.y);

    return Quad(center, size);
}

class Node
{
public:
    size_t children;
    size_t next;
    glm::vec2 pos;
    Quad quad;
    float mass;

    Node(size_t next, const Quad &quad)
        : children(0),
          next(next),
          pos(0.0f, 0.0f),
          mass(0.0f),
          quad(quad)
    {
    }

    bool is_leaf() const noexcept { return children == 0; }
    bool is_branch() const noexcept { return children != 0; }
    bool is_empty() const noexcept { return mass == 0.0f; }
};

class Quadtree
{
public:
    static constexpr size_t ROOT = 0;
    const float t_2;
    const float e_2;
    std::vector<Node> nodes;
    std::vector<size_t> parents;

    Quadtree(float theta, float epsilon)
        : t_2(theta * theta),
          e_2(epsilon * epsilon),
          nodes(),
          parents() {}

    void clear(const Quad &quad)
    {
        nodes.clear();
        parents.clear();
        nodes.emplace_back(0, quad);
    }

    void insert(const glm::vec2 &pos, float mass)
    {
        size_t node = ROOT;

        while (nodes[node].is_branch())
        {
            const int quadrant = nodes[node].quad.find_quadrant(pos);
            node = nodes[node].children + quadrant;
        }

        if (nodes[node].is_empty())
        {
            nodes[node].pos = pos;
            nodes[node].mass = mass;
            return;
        }

        const auto [p, m] = std::tie(nodes[node].pos, nodes[node].mass);
        if (pos == p)
        {
            nodes[node].mass += mass;
            return;
        }

        while (true)
        {
            const size_t children = subdivide(node);
            const int q1 = nodes[node].quad.find_quadrant(p);
            const int q2 = nodes[node].quad.find_quadrant(pos);

            if (q1 != q2)
            {
                const size_t n1 = children + q1;
                const size_t n2 = children + q2;
                nodes[n1].pos = p;
                nodes[n1].mass = m;
                nodes[n2].pos = pos;
                nodes[n2].mass = mass;
                return;
            }
            node = children + q1;
        }
    }

    size_t subdivide(size_t node)
    {
        parents.push_back(node);
        const size_t children = nodes.size();
        nodes[node].children = children;

        const std::array<size_t, 4> nexts = {
            children + 1,
            children + 2,
            children + 3,
            nodes[node].next,
        };

        const auto quads = nodes[node].quad.subdivide();
        for (size_t i = 0; i < 4; ++i)
        {
            nodes.emplace_back(nexts[i], quads[i]);
        }

        return children;
    }

    void propagate()
    {
        for (auto it = parents.rbegin(); it != parents.rend(); ++it)
        {
            const size_t node = *it;
            const size_t i = nodes[node].children;

            glm::vec2 pos_sum(0.0f);
            float mass_sum = 0.0f;

            for (size_t j = 0; j < 4; ++j)
            {
                pos_sum += nodes[i + j].pos * nodes[i + j].mass;
                mass_sum += nodes[i + j].mass;
            }

            nodes[node].pos = pos_sum / mass_sum;
            nodes[node].mass = mass_sum;
        }
    }

    glm::vec2 acc(const glm::vec2 &pos) const
    {
        glm::vec2 acceleration(0.0f);
        size_t node = ROOT;

        while (true)
        {
            const Node &n = nodes[node];
            const glm::vec2 d = n.pos - pos;
            const float d_sq = glm::dot(d, d);

            if (n.is_leaf() || (n.quad.size * n.quad.size < d_sq * t_2))
            {
                const float denom = (d_sq + e_2) * std::sqrt(d_sq);
                if (denom > 0.0f)
                {
                    acceleration += d * std::min(n.mass / denom, std::numeric_limits<float>::max());
                }

                if (n.next == 0)
                    break;

                node = n.next;
            }
            else
            {
                node = n.children;
            }
        }

        return acceleration;
    }
};

#endif