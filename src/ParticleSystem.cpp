#include "ParticleSystem.h"

#include <omp.h>

#include <iostream>
#include <random>


void fixedPointFn(Particle* particle, const float& dt) 
{
    particle->forces = glm::vec3(0.0f); 
    particle->velocity = glm::vec3(0.0f);
}

Particle FixedPoint(const glm::vec3& position)
{
    return Particle{position, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, fixedPointFn};
}

void sphereObstacleFn(Particle* particle, const float& dt)
{
    particle->forces = glm::vec3(0.0f); 
    particle->velocity = glm::vec3(0.0f);
}

Link Spring(Particle* p1, 
            Particle* p2,
            const float& k,
            const float& restLength) 
{
    return Link{p1, p2, k, restLength, 0.0f, springFn};
}

Link Damper(Particle* p1, 
            Particle* p2,
            const float& z) 
{
    return Link{p1, p2, 0.0f, 0.0f, z, damperFn};
}

Link SpringDamper(Particle* p1, 
                  Particle* p2,
                  const float& k,
                  const float& restLength,
                  const float& z) {
    return Link{p1, p2, k, restLength, z, springDamperFn};
}

float K(const float& k, const float& fe, const float& m)
{
    return fe * fe * k / m;
}

float Z(const float& z, const float& fe, const float& m)
{
    return fe * z / m;
}

float ObstacleStiffness(const float& fe)
{
    return fe * fe;
}

void springFn(const Link* link) 
{
    glm::vec3 diff = link->p1->position - link->p2->position;
    float deltaLength = glm::length(diff) - link->restLength;
    glm::vec3 f = -link->k * deltaLength * glm::normalize(diff);
    link->p1->forces += f;
    link->p2->forces -= f;
}

void damperFn(const Link* link) 
{
    glm::vec3 f = -link->z * (link->p1->velocity - link->p2->velocity);
    link->p1->forces += f;
    link->p2->forces -= f;
}


void springDamperFn(const Link* link) {
    springFn(link);
    damperFn(link);
}

void leapFrog(Particle* particle, const float& dt)
{
    particle->velocity += particle->forces / particle->mass * dt;
    particle->position += particle->velocity * dt;
    particle->forces = glm::vec3(0);
}

void Obstacle::update(Particle* particle) const
{
    float dist = size - distanceFn(position, particle->position);
    if (dist > 0.0f)
    {
        glm::vec3 normal = normalFn(position, particle->position);
        particle->forces += stiffness * dist * normal;
    }    
}

void massSpringSolver(ParticleSystem& partSys, const double &deltaTime)
{
    for (const auto& link : partSys.links) {
        link.update(&link);
    }

    for (auto& part : partSys.particles) {
        part.update(&part, deltaTime);
    }
}

void massSpringGravitySolver(ParticleSystem& partSys, const double &deltaTime)
{
    for (const auto& link : partSys.links) {
        link.update(&link);
    }

    for (auto& part : partSys.particles) {
        part.forces += glm::vec3(0, -gravity, 0);
        part.update(&part, deltaTime);
    }
}

void massSpringGravityWindSolver(ParticleSystem& partSys, const double &deltaTime)
{
    #pragma omp parallel for num_threads(omp_get_max_threads())
    for (const auto& link : partSys.links) {
        link.update(&link);
    }

    #pragma omp parallel for num_threads(omp_get_max_threads())
    for (auto& part : partSys.particles) {
        for (const auto& obstacle : partSys.obstacles)
            obstacle.update(&part);

        part.forces += glm::vec3(0, -gravity, 0);
        part.forces += wind;
        
        part.update(&part, deltaTime);
    }
}

float& getGravity() 
{ 
    return gravity; 
}

glm::vec3& getWind() 
{ 
    return wind; 
}


void InitClothFromMesh(ParticleSystem& partSys,
                       const std::vector<Vertex> vertices,
                       const uint32_t& divisionsW, 
                       const uint32_t& divisionsH,
                       const float& fe) {
    float k = 0.2f;
    float z = 0.03f;

    partSys.links.clear();
    partSys.particles.clear();

    uint32_t vtxCountW = divisionsW + 1;
    uint32_t vtxCountH = divisionsH + 1;
    for (int y = 0; y < vtxCountH - 1 ; y++) {
        for (int x=0 ; x < vtxCountW ; ++x) {
            auto part = Particle{vertices[y * (vtxCountW) + x].position};
            part.mass = 1.0f;
            part.update = leapFrog;
            partSys.particles.push_back(part);
        }
    }
    // Top line is only composed of fixed points
    for (int x=0 ; x < vtxCountW ; ++x) {
        partSys.particles.push_back(FixedPoint(vertices[(vtxCountH - 1) * vtxCountW + x].position));
    }

    auto addLink = [&](const uint32_t& i1, const uint32_t& i2) {
        auto& part1 = partSys.particles[i1];
        auto& part2 = partSys.particles[i2];
        glm::vec3 diff = part1.position - part2.position;

        partSys.links.push_back(SpringDamper(
            &part1, &part2,
            K(k, fe, 1.0f), glm::length(diff), 
            Z(z, fe, 1.0f)));
    };

    int index = 0;
    glm::vec3 diff;
    for (int y=0 ; y < vtxCountH ; ++y) {
        for (int x=0 ; x < vtxCountW ; ++x) {
            // Bending
            if (y > 1) 
            {
                int downIndex = index - vtxCountW * 2;
                addLink(index, downIndex);

                if (x < vtxCountW - 2) 
                {
                    int diagDownIndex = downIndex + 2;
                    addLink(index, diagDownIndex);
                }
            }
            if (x < vtxCountW - 2) 
            {
                int sideIndex = index + 2;
                addLink(index, sideIndex);

                if (y < vtxCountH - 2)
                {
                    int diagUpIndex = sideIndex + vtxCountW * 2;
                    addLink(index, diagUpIndex);
                } 
            }

            // Shearing
            if (y > 0) 
            {
                int downIndex = index - vtxCountW;
                addLink(index, downIndex);

                if (x < vtxCountW - 1) 
                {
                    int diagDownIndex = downIndex + 1;
                    addLink(index, diagDownIndex);
                }
            }
            if (x < vtxCountW - 1) 
            {
                int sideIndex = index + 1;
                addLink(index, sideIndex);

                if (y < vtxCountH - 1)
                {
                    int diagUpIndex = sideIndex + vtxCountW;
                    addLink(index, diagUpIndex);
                } 
            }

            index++;
        }
    }
}
