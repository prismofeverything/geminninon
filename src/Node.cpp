#include "Node.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Color.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"
#include <vector>

using namespace ci;
using namespace std;

#define TAU 6.2831853071795862f

Node::Node()
{
}

Node::Node( float rad, Vec3f pos, Vec3f vel, float mas, Vec3f col, float theta, float amp, float ideal, float damp, float freq )
{
    radius = rad;
    position = pos;
    velocity = vel;
    mass = mas;
    color = col;
    phase = theta;
    amplitude = amp;
    damping = damp;
    idealZ = ideal;
    lateral = 0.0f;
    baselevel = 0.7f;
    level = baselevel * 0.5f;
    inertia = 0.0f;
    frequency = freq; 
    scale = 0;
}

void Node::addNeighbors( vector<uint32_t> const& other ) 
{
    neighbors.insert( neighbors.end(), other.begin(), other.end() );
    lateral = 1.0 / neighbors.size();
}

void Node::impact( vector<Node> & nodes, float length, Vec3f newColor ) {
    velocity += Vec3f( 0.0f, 0.0f, math<float>::log( length ) * 3.0 );
    color = newColor;

    for ( vector<uint32_t>::iterator index = neighbors.begin(); index != neighbors.end(); index++ ) {
        nodes[*index].color = newColor;
    }
}

float Node::advance() {
    float factor = math<float>::pow( 1.07, position[2] - idealZ );
    bool positive = level > 0;
    inertia -= level * factor * frequency; 
    level += inertia;

    if ( level > baselevel ) {
        level = baselevel;
    } else if ( level < -baselevel ) {
        level = -baselevel;
    }

    if ( positive != level > 0 ) {
        scale = ( position[2] - idealZ ) * 0.1f + 0.05f;
        if ( scale < 0 ) scale = 0;
    }

    return level * amplitude * scale;
}

void Node::changeHueSaturation( float hue, float saturation )
{
    color[0] = hue;
    color[1] = saturation;
}

void Node::update( vector<Node> const& nodes )
{
    Vec3f forces = Vec3f::zero();
    Vec3f colorForce = Vec3f::zero();
    Vec3f colorComponent;

    for ( vector<uint32_t>::iterator index = neighbors.begin(); index != neighbors.end(); index++ ) {
        forces += nodes[*index].position - position;

        colorComponent = nodes[*index].color - color;
        colorComponent[0] = math<float>::sin( colorComponent[0] * TAU * 0.1f); // hue is radial
        colorForce += colorComponent;
    }

    colorForce *= lateral * 0.001f;
    colorForce[2] = 0.0f; // not worried about brightness here
    color += colorForce;
    while ( color[0] < 0.0f ) { color[0] += 1.0f; }
    while ( color[0] > 1.0f ) { color[0] -= 1.0f; }

    velocity += Vec3f( 0.0, 0.0, forces[2] + ((idealZ - position[2])*0.1f) ) / mass;
    velocity *= damping;
}

void Node::draw()
{
    position += velocity;
    float ratio = (position[2] - idealZ) * 0.15f + 1.5f;
    Color colorcolor = Color( CM_HSV, color );

    glColor4f( colorcolor.r*ratio, colorcolor.g*ratio, colorcolor.b*ratio, 0.9f );
    gl::drawSphere( position, radius, 8 );
}
