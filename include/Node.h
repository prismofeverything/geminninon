#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include <vector>

class Node {
 public:
    Node();
    Node( float rad, ci::Vec3f pos, ci::Vec3f vel, float mas, ci::Vec3f col, float theta, float amp, float ideal, float damp );
    void update( std::vector<Node> const& nodes );
    void draw();
    void addNeighbors( std::vector<uint32_t> const& other );
    void changeHueSaturation( float hue, float saturation );

    ci::Vec3f position;
    ci::Vec3f velocity;
    ci::Vec3f color;

    float radius;
    float mass;
    float phase;
    float amplitude;
    float damping; 
    float idealZ;
    float lateral;

    std::vector<uint32_t> neighbors;
};
