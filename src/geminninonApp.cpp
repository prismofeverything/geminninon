#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Camera.h"
#include "cinder/CinderMath.h"
#include "cinder/Cinder.h"
#include "cinder/audio/Output.h"
#include "cinder/audio/Callback.h"
#include "cinder/audio/PcmBuffer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "Kinect.h"
#include "RtMidi.h"
#include "NodeSystem.h"

using namespace ci;
using namespace ci::app;
using std::vector;
using std::string;

#define GRANULARITY 53
#define TAU 6.2831853071795862f

class geminninonApp : public AppBasic {
  public:
	void prepareSettings( Settings *settings );
	void setup();
	void update();
	void draw();

    void midiIn( double deltatime, std::vector<unsigned char> *message, void *userData );
	void keyDown( KeyEvent event );	
	void keyUp( KeyEvent event );	
	void mouseDown( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	void mouseMove( MouseEvent event );	
	void mouseDrag( MouseEvent event );	

    Color randomColor();

    // system
    NodeSystem * system;

    // viewport
    CameraPersp camera;
    Quatf rotation;
    Vec3f eye, towards, up;
    Color background;
    Vec3f changeColor;

    int width;
    int height;

    // kinect
	Kinect kinect;
    bool kinectEnabled;
	gl::Texture kinectDepth;
	float kinectTilt, kinectScale;
	float XOff, mYOff;
    int kinectWidth, kinectHeight;
    Vec3f kinectColor;

    // input
    char key;
    bool keyIsDown;
    bool mouseIsDown;
    Vec2i mousePosition;
    Vec2f mouseVelocity;
};

void geminninonApp::prepareSettings( Settings *settings )
{
    Rand::randomize();

    width = 800;
    height = width * 0.866;
	settings->setWindowSize( width, height );
	settings->setFrameRate( 40.0f );
}

void geminninonApp::midiIn( double deltatime, std::vector<unsigned char> *message, void *userData )
{
    unsigned char type = (*message)[0];
    unsigned char key = message->size() > 1 ? (*message)[1] : 0;
    unsigned char velocity = message->size() > 2 ? (*message)[2] : 0;

    if ( type == 176 ) { // control
        if ( key == 19 ) {
            kinectColor[2] = velocity / 127.1;
        } else if ( key == 18 ) {
            kinectColor[1] = velocity / 127.1;
        } else if ( key == 80 ) {
            kinectColor[0] = velocity / 127.1;
        } else if ( key == 74 ) {
            system->mass( 5.0 + velocity );
        }
    } else if ( type == 144 ) { // key

    } else if ( type == 224 ) { // pitch

    }
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
    system->mouseImpact( mousePosition, mouseVelocity, changeColor );
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
    system->mouseImpact( mousePosition, mouseVelocity, changeColor );
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

    rotation.w = -0.74f;
    eye = Vec3f( 0.0f, 0.0f, 300.0f );
    towards = Vec3f::zero();
    up = Vec3f::yAxis();
	camera.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 2000.0f );

    kinectEnabled = Kinect::getNumDevices() > 0;
    if ( kinectEnabled ) {
        kinectTilt = -13.0f;
        kinect = Kinect( Kinect::Device() );
        kinectWidth = 640;
        kinectHeight = 480;
        kinectDepth = gl::Texture( kinectWidth, kinectHeight );
        kinectColor = Vec3f( 0.0f, 0.0f, 0.0f );
    }

    system = new NodeSystem();
    system->addNodes( GRANULARITY, GRANULARITY );
    system->establishNeighborhoods();

    audio::Output::play( audio::createCallback( system, &NodeSystem::generateAudio, true ) );

    gl::enableDepthRead();
    gl::enableDepthWrite();
}

void geminninonApp::update()
{
    if ( kinectEnabled ) {
        if( kinect.checkNewDepthFrame() ) {
            kinectDepth = kinect.getDepthImage();
            kinectDepth.setFlipped(true);
        }

        if( kinectTilt != kinect.getTilt() ) {
            kinect.setTilt( kinectTilt );
        }
    }

    camera.lookAt( eye, towards, up );
    gl::setMatrices( camera );
    gl::rotate( rotation );

    if ( keyIsDown ) {
        if ( key == 32 ) { // space
            background = randomColor();
            system->changeHueSaturation( Rand::randFloat(), Rand::randFloat() );
        } else if ( key == 'j' ) { // left
            rotation.v[0] += 1.0;
        } else if ( key == 'i' ) { // up
            rotation.w += 0.01;
            if ( rotation.w > 1 ) { rotation.w -= 2.0f; }
        } else if ( key == 'l' ) { // right
            rotation.v[0] -= 1.0;
        } else if ( key == 'k' ) { // down
            rotation.w -= 0.01;
            if ( rotation.w <= -1 ) { rotation.w += 2; }
        }
    }

    system->update();
}

void geminninonApp::draw()
{
    gl::clear( background );

    system->draw();

    if ( kinectEnabled ) {
        Color kcolor = Color( CM_HSV, kinectColor );
        glColor4f( kcolor.r, kcolor.g, kcolor.b, 0.7f );
        gl::draw( kinectDepth, Vec2f( -kinectWidth * 0.5, -kinectHeight * 0.5 ) );
    }
}

AppBasic *geminninon;

void midiIn( double deltatime, std::vector<unsigned char> *message, void *userData )
{
    ((geminninonApp *) geminninon)->midiIn( deltatime, message, userData );
}

int main( int argc, char * const argv[] ) {								
    AppBasic::prepareLaunch();							
    geminninon = new geminninonApp();								
    Renderer *ren = new RendererGl();							
    std::string title("yellow");

    bool midiEnabled = true;
    RtMidiIn * midiin = 0;

    try {
        midiin = new RtMidiIn();
    } catch ( RtError &error ) {
        error.printMessage();
        midiEnabled = false;
    }

    if ( midiEnabled ) {
        try {
            midiin->openPort( 0 );
        } catch ( RtError &error ) {
            error.printMessage();
            delete midiin;
        }

        midiin->setCallback( &midiIn );
        midiin->ignoreTypes( false, false, true );
    }

    AppBasic::executeLaunch( geminninon, ren, title.c_str(), argc, argv );
    AppBasic::cleanupLaunch();								

    if ( midiEnabled ) {
        delete midiin;
    }

    return 0;															
}


