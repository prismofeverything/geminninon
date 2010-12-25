#include "Node.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include <vector>

using namespace ci;
using namespace std;

Node::Node()
{
}

Node::Node( float rad, Vec3f pos, Vec3f vel, float mas, Vec3f col, float theta, float amp, float ideal, float damp )
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
}

void Node::addNeighbors( vector<uint32_t> const& other ) 
{
    neighbors.insert( neighbors.end(), other.begin(), other.end() );
    lateral = 1.0 / neighbors.size();
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
    for ( vector<uint32_t>::iterator index = neighbors.begin(); index != neighbors.end(); index++ ) {
        forces += nodes[*index].position - position;
        colorForce += nodes[*index].color - color;
    }

    colorForce *= lateral * 0.1f;
    colorForce[2] = 0.0f;
    color += colorForce;

    velocity += Vec3f( 0.0, 0.0, forces[2] + ((idealZ - position[2])*0.1f) ) / mass;
    velocity *= damping;
}

void Node::draw()
{
    float ratio = (idealZ - position[2]) * 0.2f + 1.0f;
    position += velocity;
    Color colorcolor = Color( CM_HSV, color );
    glColor4f( colorcolor.r*ratio, colorcolor.g*ratio, colorcolor.b*ratio, 0.9f );
    gl::drawSphere( position, 10.0, 16 );
}
