#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "Base/Mesh.h"

#include <glm/glm.hpp>

#include <functional>
#include <vector>


struct Particle;
struct Link;
struct Obstacle;

using UpdateParticleFn = std::function<void(Particle*, const float& dt)>;
using DistanceFn       = std::function<float(const glm::vec3&, const glm::vec3&)>;
using NormalFn         = std::function<glm::vec3(const glm::vec3&, const glm::vec3&)>;
using UpdateLinkFn     = std::function<void(const Link*)>;


struct Particle
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 forces;
    float mass = 1.0;

    UpdateParticleFn update;
};

// Particle creation
void fixedPointFn(Particle* particle, const float& dt);
Particle FixedPoint(const glm::vec3& position);

// Particle update
void leapFrog(Particle* particle, const float& dt);

struct Link
{
    Particle *p1, *p2;
    float k;
    float restLength;
    float z;

    UpdateLinkFn update;
};

// Link creation
Link Spring(Particle* p1, 
            Particle* p2,
            const float& k=1.0f,
            const float& restLength=0.0f);
Link Damper(Particle* p1, 
            Particle* p2,
            const float& z);
Link SpringDamper(Particle* p1, 
                  Particle* p2,
                  const float& k=1.0f,
                  const float& restLength=0.0f,
                  const float& z=1.0f);

struct Obstacle
{
    glm::vec3 position;
    float size;
    float stiffness;
    DistanceFn distanceFn;
    NormalFn normalFn;

    void update(Particle* particle) const;
};


inline float euclidianNorm(const glm::vec3& p1, const glm::vec3& p2)
{
    return glm::distance(p1, p2);
}

inline float infiniteNorm(const glm::vec3& p1, const glm::vec3& p2)
{
    return std::max(std::abs(p1.x - p2.x), 
                    std::max(std::abs(p1.y - p2.y),
                             std::abs(p1.z - p2.z)));
}

inline glm::vec3 sphericalNormal(const glm::vec3& p1, const glm::vec3& p2)
{
    return glm::normalize(p2 - p1);
}

inline glm::vec3 cubeNormal(const glm::vec3& p1, const glm::vec3& p2)
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;

    float adx = std::abs(dx);
    float ady = std::abs(dy);
    float adz = std::abs(dz);

    if (adx >= ady)
        if (adx >= adz)
            return glm::vec3(1.0f, 0.0f, 0.0f) * (dx >= 0.0f ? 1.0f : -1.0f);
        else
            return glm::vec3(0.0f, 0.0f, 1.0f) * (dz >= 0.0f ? 1.0f : -1.0f);
    else if (ady > adz)
        return glm::vec3(0.0f, 1.0f, 0.0f) * (dy >= 0.0f ? 1.0f : -1.0f);
    else
        return glm::vec3(0.0f, 0.0f, 1.0f) * (dz >= 0.0f ? 1.0f : -1.0f);
}


inline glm::vec2 smoothMin(const float& a, const float& b, const float& k )
{
    float h = std::max(k - std::abs(a - b), 0.0f) / k;
    float m = h * h * h * 0.5f;
    float s = m * k * (1.0f / 3.0f); 
    return (a < b) ? glm::vec2(a - s, m) : glm::vec2(b - s, 1.0 - m);
}


inline Obstacle SphereObstacle(const glm::vec3& position, const float& radius, const float& stiffness)
{
    return Obstacle{position, radius, stiffness, euclidianNorm, sphericalNormal};
}

inline Obstacle CubeObstacle(const glm::vec3& position, const float& width, const float& stiffness)
{
    return Obstacle{position, width, stiffness, infiniteNorm, cubeNormal};
}


// Link parameters
float K(const float& k, const float& fe, const float& m);
float Z(const float& z, const float& fe, const float& m);
float ObstacleStiffness(const float& fe);

// Update link
void springFn(const Link* link);
void damperFn(const Link* link);
void springDamperFn(const Link* link);


struct ParticleSystem 
{
    std::vector<Particle> particles;
    std::vector<Link> links;
    std::vector<Obstacle> obstacles;
};

// Solvers
void massSpringSolver(ParticleSystem& partSys, const double& deltaTime);
void massSpringGravitySolver(ParticleSystem& partSys, const double& deltaTime);
void massSpringGravityWindSolver(ParticleSystem& partSys, const double& deltaTime);

// Utils
static float gravity = 9.81f;
float& getGravity();

static glm::vec3 wind = {12.0f, 0.0f, 5.0f};
glm::vec3& getWind();



void InitClothFromMesh(ParticleSystem& partSys,
                       const std::vector<Vertex> vertices,
                       const uint32_t& divisionsW, 
                       const uint32_t& divisionsH,
                       const float& fe);

#endif
