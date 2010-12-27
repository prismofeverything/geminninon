#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "NodeSystem.h"

using namespace ci;
using namespace ci::app;
using std::vector;

#define GRANULARITY 50

class geminninonApp : public AppBasic {
  public:
	void prepareSettings( Settings *settings );
	void setup();
	void update();
	void draw();

	void keyDown( KeyEvent event );	
	void mouseDown( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	void mouseMove( MouseEvent event );	
	void mouseDrag( MouseEvent event );	

    Color randomColor();

    NodeSystem system;

    CameraPersp camera;
    Quatf rotation;
    Vec3f eye, towards, up;
    Color background;
    Vec3f changeColor;

    bool down;
    Vec2i mousePosition;
    Vec2f mouseVelocity;
};

void geminninonApp::prepareSettings( Settings *settings )
{
    Rand::randomize();
	settings->setWindowSize( 800, 800*0.866 );
	settings->setFrameRate( 40.0f );
}

void geminninonApp::keyDown( KeyEvent event )
{
    background = randomColor();
    system.changeHueSaturation( Rand::randFloat(), Rand::randFloat() );
}

void geminninonApp::mouseDown( MouseEvent event )
{
    down = true;
    changeColor = Vec3f( Rand::randFloat(), Rand::randFloat(), 0.5 );
    system.mouseImpact( mousePosition, mouseVelocity, changeColor );
}

void geminninonApp::mouseUp( MouseEvent event )
{
    down = false;
}

void geminninonApp::mouseMove( MouseEvent event )
{
    mouseVelocity = event.getPos() - mousePosition;
    mousePosition = event.getPos();
}

void geminninonApp::mouseDrag( MouseEvent event )
{
    mouseMove( event );
    system.mouseImpact( mousePosition, mouseVelocity, changeColor );
}

Color geminninonApp::randomColor()
{
    return Color( CM_HSV, Vec3f( Rand::randFloat(), Rand::randFloat(), Rand::randFloat() ) );
}

void geminninonApp::setup()
{
    background = randomColor();
    down = false;
    eye = Vec3f( 0.0f, 0.0f, 300.0f );
    towards = Vec3f::zero();
    up = Vec3f::yAxis();
	camera.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 2000.0f );

    system.addNodes( GRANULARITY, GRANULARITY );
    system.establishNeighborhoods();
}

void geminninonApp::update()
{
    camera.lookAt( eye, towards, up );
    gl::setMatrices( camera );
    gl::rotate( rotation );

    system.update();
    // if ( down ) {
    //     system.mouseImpact( mousePosition, mouseVelocity, changeColor );
    // }
}

void geminninonApp::draw()
{
    gl::clear( background );
    gl::enableDepthRead();
    gl::enableDepthWrite();

    system.draw();
}


CINDER_APP_BASIC( geminninonApp, RendererGl )
