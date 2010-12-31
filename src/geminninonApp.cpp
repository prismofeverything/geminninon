#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "RtMidi.h"
#include "NodeSystem.h"

using namespace ci;
using namespace ci::app;
using std::vector;
using std::string;

#define GRANULARITY 50
#define TAU 6.2831853071795862f

struct midi {
    unsigned char type;
    unsigned char key;
    unsigned char velocity;
    float stamp;
};

// global midi variables
bool midiReady;
std::vector<midi> midiEvents;

class geminninonApp : public AppBasic {
  public:
	void prepareSettings( Settings *settings );
	void setup();
	void update();
	void draw();

	void keyDown( KeyEvent event );	
	void keyUp( KeyEvent event );	
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

    char key;
    bool keyIsDown;
    bool mouseIsDown;
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
    key = event.getChar();
    keyIsDown = true;
}

void geminninonApp::keyUp( KeyEvent event )
{
    keyIsDown = false;
}

void geminninonApp::mouseDown( MouseEvent event )
{
    mouseIsDown = true;
    changeColor = Vec3f( Rand::randFloat(), Rand::randFloat(), 0.5 );
    system.mouseImpact( mousePosition, mouseVelocity, changeColor );
}

void geminninonApp::mouseUp( MouseEvent event )
{
    mouseIsDown = false;
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
    mouseIsDown = false;
    keyIsDown = false;

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

    if ( keyIsDown ) {
        if ( key == 32 ) { // space
            background = randomColor();
            system.changeHueSaturation( Rand::randFloat(), Rand::randFloat() );
        } else if ( key == 'j' ) { // left
            rotation.v[0] += 1.0;
        } else if ( key == 'i' ) { // up
            rotation.w += 0.01;
            if ( rotation.w > 1 ) { rotation.w -= 2; }
        } else if ( key == 'l' ) { // right
            rotation.v[0] -= 1.0;
        } else if ( key == 'k' ) { // down
            rotation.w -= 0.01;
            if ( rotation.w < -1 ) { rotation.w += 2; }
        }
    }

    if ( midiReady ) {
        background = randomColor();
        system.changeHueSaturation( Rand::randFloat(), Rand::randFloat() );
        midiReady = false;
    }

    system.update();
}

void geminninonApp::draw()
{
    gl::clear( background );
    gl::enableDepthRead();
    gl::enableDepthWrite();

    system.draw();
}

void midiIn( double deltatime, std::vector<unsigned char> *message, void *userData )
{
    midiReady = true;
    midi event;
    event.stamp = deltatime;
    event.type = (*message)[0];

    if ( message->size() > 1 ) {
        event.key = (*message)[1];

        if ( message->size() > 2 ) {
            event.velocity = (*message)[2];
        }
    }
    
    midiEvents.push_back( event );
}

int main( int argc, char * const argv[] ) {								
    AppBasic::prepareLaunch();								
    AppBasic *app = new geminninonApp();								
    Renderer *ren = new RendererGl();							
    std::string title("yellow");

    midiReady = false;
    RtMidiIn * midiin = 0;

    try {
        //        midiin = boost::shared_ptr<RtMidiIn>(new RtMidiIn());
        midiin = new RtMidiIn();
    }
    catch ( RtError &error ) {
        error.printMessage();
        exit( EXIT_FAILURE );
    }

    try {
        midiin->openPort( 0 );
    }
    catch ( RtError &error ) {
        error.printMessage();
        goto cleanup;
    }

    midiin->setCallback( &midiIn );
    midiin->ignoreTypes( false, false, true );

    AppBasic::executeLaunch( app, ren, title.c_str(), argc, argv );	
    AppBasic::cleanupLaunch();								

 cleanup:    
    delete midiin;

    return 0;															
}


