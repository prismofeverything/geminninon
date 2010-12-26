#include "NodeSystem.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include <vector>
#include <algorithm>

using namespace ci;
using namespace std;

NodeSystem::NodeSystem()
{
    dim = Vec2i(0, 0);
    dispersal = 20.0;
}

Vec2f NodeSystem::disperse( const Vec2i & index )
{
    return (index - dim*0.5f) * dispersal;
}

int NodeSystem::nodeUnder( const Vec2f & over )
{
    Vec2i index = (over / (dispersal * 1.55)) * Vec2f( 1.0, 1.1547 ) + Vec2i(1, 1);
    if ( index[0] < 0 ) { index[0] = 0; }
    if ( index[1] < 0 ) { index[1] = 0; }
    if ( index[0] >= dim[0] ) { index[0] = dim[0] - 1; }
    if ( index[1] >= dim[1] ) { index[1] = dim[1] - 1; }
    uint32_t into = (dim[1]-index[1])*dim[0] + index[0];

    return into;
}

void NodeSystem::addNodes( int width, int height )
{
    dim[0] = width;
    dim[1] = height;

    dispersal = 540.0 / max(width, height);
    float hue = Rand::randFloat();
    float offset = 0.0f;
    float hexagon = 0.866f;

    for ( int h = 0; h < height; h++ ) {
        offset = offset == 0.0f ? 0.5f : 0.0f;
        for ( int w = 0; w < width; w++ ) {
            Node node = Node( 9.0,
                              Vec3f( (w+offset-width*0.5)*dispersal, 
                                     (h-height*0.5)*dispersal*hexagon, 
                                     -20.0f + Rand::randFloat( 10.0f ) - 5.0f), 
                              Vec3f::zero(), 
                              20.0f, // + Rand::randFloat( 50.0f ), 
                              Vec3f( hue, 0.5f, 0.5f ), //Vec3f( Rand::randFloat(), Rand::randFloat(), Rand::randFloat() ), 
                              0.0f, 0.4f, -20.0f, 0.99f );

            nodes.push_back( node );
        }
    }
}

void NodeSystem::establishNeighborhoods()
{
    const int width = dim[0];
    const int height = dim[1];

    vector<uint32_t> indexes;
    uint32_t index = 0;
    int row = 0;
    int around = 0;
    bool offset = false;
    bool hexagonalTiling = false;

    for ( int h = 0; h < height; h++ ) {
        offset = !offset;

        for ( int w = 0; w < width; w++ ) {
            index = h * width + w;
            indexes.clear();

            for ( int vert = -width; vert <= width; vert += width ) {
                for ( int horz = -1; horz <= 1; horz++ ) {
                    row = horz + w;
                    around = index + vert + horz;
                    hexagonalTiling = vert == 0 || (offset ? horz >= 0 : horz <= 0);

                    if ( around >= 0 && around < nodes.size() && around != index 
                         && row >= 0 && row < width 
                         && hexagonalTiling ) {
                        indexes.push_back( around );
                    }
                }
            }

            nodes[index].addNeighbors( indexes );
        }
    }
}

void NodeSystem::mouseImpact( const Vec2i & mouse, const Vec2f & velocity, const Vec3f & color )
{
    int under = nodeUnder( mouse );
    nodes[under].velocity += Vec3f( 0.0f, 0.0f, 10.0f );
    nodes[under].color = color;

    for ( vector<uint32_t>::iterator index = nodes[under].neighbors.begin(); index != nodes[under].neighbors.end(); index++ ) {
        nodes[*index].color = color;
    }
}

void NodeSystem::changeHueSaturation( float hue, float saturation )
{
    for ( vector<Node>::iterator node = nodes.begin(); node != nodes.end(); node++ ) {
        node->changeHueSaturation( hue, saturation );
    }
}

void NodeSystem::update()
{
    for ( vector<Node>::iterator node = nodes.begin(); node != nodes.end(); node++ ) {
        node->update( nodes );
    }
}

void NodeSystem::draw()
{
    for ( vector<Node>::iterator node = nodes.begin(); node != nodes.end(); node++ ) {
        node->draw();
    }
}
