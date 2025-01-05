#pragma once

#include <glm.hpp>
#include <vector>
#include <array>
#include "Collision.h"

static glm::vec3 temp = { 0,0,0 };


struct Simplex
{
private:
    std::array<glm::vec3, 4> points;
    unsigned _size;

public:
    Simplex() : points({ temp, temp, temp, temp }), _size(0) {}

    Simplex& operator = (std::initializer_list<glm::vec3> list)
    {
        for (auto v = list.begin(); v != list.end(); v++)
        {
            points[std::distance(list.begin(), v)] = *v;
        }
        _size = list.size();

        return *this;
    }

    void pushFront(glm::vec3 point)
    {
        points = { point, points[0], points[1], points[2] };
        _size = std::min(_size + 1, 4u);
    }

    glm::vec3& operator[](unsigned i) { return points[i]; }
    unsigned size() const { return _size; }

    auto begin() const { return points.begin(); }
    auto end() const { return points.end() - (4 - _size); }
};

static glm::vec3 Support(const CollisionMesh* colliderA, const CollisionMesh* colliderB, glm::vec3 direction)
{
    return colliderA->FindFuhrestPoint(direction) - colliderB->FindFuhrestPoint(-direction);
}


static bool SameDirection(const glm::vec3& direction, const glm::vec3& ao)
{
    return glm::dot<3, float, glm::qualifier::highp>(direction, ao) > 0;
}

static bool Line(Simplex& points, glm::vec3& direction)
{
    glm::vec3 a = points[0];
    glm::vec3 b = points[1];

    glm::vec3 ab = b - a;
    glm::vec3 ao = -a;

    if (SameDirection(ab, ao)) {
        direction = glm::cross(glm::cross(ab, ao), ab);
    }

    else {
        points = { a };
        direction = ao;
    }

    return false;
}

static bool Triangle(Simplex& points, glm::vec3& direction)
{
    glm::vec3 a = points[0];
    glm::vec3 b = points[1];
    glm::vec3 c = points[2];

    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 ao = -a;

    glm::vec3 abc = glm::cross(ab, ac);

    if (SameDirection(glm::cross(abc, ac), ao)) {
        if (SameDirection(ac, ao)) {
            points = { a, c };
            direction = glm::cross(glm::cross(ac, ao), ac);
        }

        else {
            return Line(points = { a, b }, direction);
        }
    }

    else {
        if (SameDirection(glm::cross(ab, abc), ao)) {
            return Line(points = { a, b }, direction);
        }

        else {
            if (SameDirection(abc, ao)) {
                direction = abc;
            }

            else {
                points = { a, c, b };
                direction = -abc;
            }
        }
    }

    return false;
}

static bool Tetrahedron(Simplex& points, glm::vec3& direction)
{
    glm::vec3 a = points[0];
    glm::vec3 b = points[1];
    glm::vec3 c = points[2];
    glm::vec3 d = points[3];

    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 ad = d - a;
    glm::vec3 ao = -a;

    glm::vec3 abc = glm::cross(ab, ac);
    glm::vec3 acd = glm::cross(ac, ad);
    glm::vec3 adb = glm::cross(ad, ab);

    if (SameDirection(abc, ao)) {
        return Triangle(points = { a, b, c }, direction);
    }

    if (SameDirection(acd, ao)) {
        return Triangle(points = { a, c, d }, direction);
    }

    if (SameDirection(adb, ao)) {
        return Triangle(points = { a, d, b }, direction);
    }

    return true;
}

static bool NextSimplex(Simplex& points, glm::vec3 direction)
{
    switch (points.size())
    {
    case 2: return Line(points, direction);
    case 3: return Triangle(points, direction);
    case 4: return Tetrahedron(points, direction);
    }

    return false;
}



static bool GJK(const CollisionMesh* colliderA, const CollisionMesh* colliderB)
{
    glm::vec3 support = Support(colliderA, colliderB, { 1,0,0 });

    Simplex points;
    points.pushFront(support);

    glm::vec3 direction = -support;

    while (true)
    {
        support = Support(colliderA, colliderB, direction);

        if (glm::dot<3, float, glm::qualifier::highp>(support, direction) <= 0)
        {
            return false;
        }

        points.pushFront(support);

        if (NextSimplex(points, direction))
        {
            return true;
        }
    }
}

struct CollisionPoints
{
    glm::vec3 A;
    glm::vec3 B;
    glm::vec3 normal;
    glm::vec3 depth;
    bool hasCollision;
};

struct Collision
{
    CollisionMesh* objA;
    CollisionMesh* objB;
    CollisionPoints points;
};