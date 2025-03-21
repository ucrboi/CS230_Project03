#ifndef SIMULATION_H
#define SIMULATION_H

#include <glm/glm.hpp>
#include <vector>
#include <limits>
#include <cmath>
#include <random>
#include <iostream>
#include <omp.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "body.h"
#include "quadtree.h"

const float THETA = 1.5;
const float EPSILON = 1.0;
extern const bool COLLISION;

class Simulation
{

public:
    int n;
    int frame;
    float dt;
    std::vector<Body> &bodies;
    Quadtree qt = Quadtree(THETA, EPSILON);
    Simulation(int n, float dt, std::vector<Body> &b) : n(n), dt(dt), frame(0), bodies(b) {};

    void step()
    {
        attract();
        if (COLLISION)
            collide();
        iterate();
        frame += 1;
    }

    void iterate()
    {
        for (auto &b : bodies)
        {
            b.update(dt);
        }
    }

    void attract()
    {
        Quad q = new_quadtree(bodies);
        qt.clear(q);

        for (const auto &b : bodies)
        {
            qt.insert(b.position, b.mass);
        }

        qt.propagate();

#pragma omp parallel for
        for (auto &b : bodies)
        {
            b.acceleration = qt.acc(b.position);
        }
    }

    void collide()
    {
        if (bodies.size() <= 1)
            return;

        // Initialize disjoint-set arrays for union-find
        std::vector<size_t> parent(bodies.size());
        std::vector<size_t> rank(bodies.size(), 0);
        for (size_t i = 0; i < bodies.size(); ++i)
            parent[i] = i;

        // Find function with path compression
        std::function<size_t(size_t)> find = [&](size_t x) -> size_t
        {
            if (parent[x] != x)
                parent[x] = find(parent[x]);
            return parent[x];
        };

        // Union function using rank for balance
        auto unite = [&](size_t x, size_t y)
        {
            x = find(x);
            y = find(y);
            if (x == y)
                return;
            if (rank[x] < rank[y])
                parent[x] = y;
            else
            {
                parent[y] = x;
                if (rank[x] == rank[y])
                    rank[x]++;
            }
        };

        // Create spatial grid
        Quad q = new_quadtree(bodies);

        // Determine grid cell size based on maximum body radius
        float max_radius = 0.0f;
        for (const auto &body : bodies)
            max_radius = std::max(max_radius, body.radius);

        float grid_cell_size = std::max(max_radius * 4.0f, q.size / 50.0f);
        int grid_width = static_cast<int>(std::ceil(q.size / grid_cell_size));
        int grid_height = grid_width;
        grid_cell_size = q.size / static_cast<float>(grid_width);

        // Create grid structure (each cell will store indices of bodies)
        std::vector<std::vector<size_t>> grid(grid_width * grid_height);

        // Lambda to map position to grid cell coordinates
        auto pos_to_cell = [&](const glm::vec2 &pos) -> std::pair<int, int>
        {
            float min_x = q.center.x - q.size / 2.0f;
            float min_y = q.center.y - q.size / 2.0f;
            int x = static_cast<int>((pos.x - min_x) / grid_cell_size);
            int y = static_cast<int>((pos.y - min_y) / grid_cell_size);
            x = std::max(0, std::min(x, grid_width - 1));
            y = std::max(0, std::min(y, grid_height - 1));
            return {x, y};
        };

        // --- Step 1: Compute grid cell for each body in parallel ---
        // We store the cell index for each body.
        std::vector<int> cellIndex(bodies.size());
#pragma omp parallel for
        for (size_t i = 0; i < bodies.size(); ++i)
        {
            auto cell = pos_to_cell(bodies[i].position);
            cellIndex[i] = cell.second * grid_width + cell.first;
        }

        // Insert bodies into the grid (done sequentially to avoid data races)
        for (size_t i = 0; i < bodies.size(); ++i)
        {
            grid[cellIndex[i]].push_back(i);
        }

        // --- Step 2: Collision detection ---
        // Accumulate collision pairs in parallel.
        std::vector<std::pair<size_t, size_t>> collisionPairs;
#pragma omp parallel
        {
            std::vector<std::pair<size_t, size_t>> localPairs;
#pragma omp for nowait
            for (size_t i = 0; i < bodies.size(); ++i)
            {
                auto cell = pos_to_cell(bodies[i].position);
                int x1 = cell.first;
                int y1 = cell.second;
                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        int nx = x1 + dx;
                        int ny = y1 + dy;
                        if (nx < 0 || nx >= grid_width || ny < 0 || ny >= grid_height)
                            continue;
                        const auto &cellBodies = grid[ny * grid_width + nx];
                        for (size_t j : cellBodies)
                        {
                            if (i >= j)
                                continue; // ensure each pair is checked once
                            const Body &b1 = bodies[i];
                            const Body &b2 = bodies[j];
                            glm::vec2 diff = b1.position - b2.position;
                            float distance_squared = glm::dot(diff, diff);
                            float merge_threshold = (b1.radius + b2.radius) * (b1.radius + b2.radius);
                            if (distance_squared <= merge_threshold)
                            {
                                localPairs.emplace_back(i, j);
                            }
                        }
                    }
                }
            }
            // Merge thread-local pairs into the global vector.
#pragma omp critical
            {
                collisionPairs.insert(collisionPairs.end(), localPairs.begin(), localPairs.end());
            }
        }

        // Process union-find sequentially over the collision pairs.
        for (const auto &pair : collisionPairs)
        {
            unite(pair.first, pair.second);
        }

        // --- Step 3: Group bodies by their set representative ---
        std::unordered_map<size_t, std::vector<size_t>> groups;
        for (size_t i = 0; i < bodies.size(); ++i)
        {
            groups[find(i)].push_back(i);
        }

        // --- Step 4: Merge groups into new bodies ---
        std::vector<Body> new_bodies;
        new_bodies.reserve(groups.size());
        for (const auto &group : groups)
        {
            const std::vector<size_t> &indices = group.second;
            if (indices.size() == 1)
            {
                new_bodies.push_back(bodies[indices[0]]);
            }
            else
            {
                // Optionally, the merge operations for different groups can be done in parallel.
                Body merged = bodies[indices[0]];
                for (size_t i = 1; i < indices.size(); ++i)
                {
                    merged = merge_bodies(merged, bodies[indices[i]]);
                }
                new_bodies.push_back(merged);
            }
        }

        // Replace old bodies with the new ones
        bodies = std::move(new_bodies);

        std::sort(bodies.begin(), bodies.end(),
                  [](const Body &a, const Body &b)
                  {
                      return glm::length2(a.position) < glm::length2(b.position);
                  });
    }
};

#endif