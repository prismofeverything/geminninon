#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rand.h"
#include <vector>
#include "Node.h"

class NodeSystem {
 public:
    NodeSystem();
    ci::Vec2f disperse( const ci::Vec2i & index );
    int nodeUnder( const ci::Vec2f & over );
    void addNodes( int wid, int hei );
    void establishNeighborhoods();
    void mouseImpact( const ci::Vec2i & mouse, const ci::Vec2f & velocity, const ci::Vec3f & color );
    void changeHueSaturation( float hue, float saturation );
    void update();
    void draw();

    ci::Vec2i dim;
    float dispersal;
    std::vector<Node> nodes;
};
