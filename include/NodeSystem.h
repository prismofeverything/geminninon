#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rand.h"
#include "cinder/audio/PcmBuffer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include <vector>
#include "Node.h"

class NodeSystem {
 public:
    NodeSystem();
    ci::Vec2f disperse( const ci::Vec2i & index );
    int nodeUnder( const ci::Vec2f & over );
    void addNodes( int wid, int hei );
    void establishNeighborhoods();
    void mouseImpact( const ci::Vec2f & mouse, const ci::Vec2f & velocity, const ci::Vec3f & color );
    void changeHueSaturation( float hue, float saturation );
    void generateAudio( uint64_t offset, uint32_t count, ci::audio::Buffer32f *buffer );
    void mass( float level );
    void update();
    void draw();

    ci::Vec2i dim;
    int total;
    float dispersal;
    std::vector<Node> nodes;
    int contributors;
    std::vector<int> focus;

    float level;
    float inertia;
};
