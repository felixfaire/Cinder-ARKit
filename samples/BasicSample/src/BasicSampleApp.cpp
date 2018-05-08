#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderARKit.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicSampleApp : public App {
  public:
    void setup() override;
    void touchesBegan( TouchEvent event ) override;
    void update() override;
    void draw() override;
    
    ARKit::SessionRef mARSession;
};

void BasicSampleApp::setup()
{
    const auto fmt = ARKit::Session::Format().configuration( ARKit::Session::TrackingConfiguration::WorldTrackingWithHorizontalPlaneDetection );
    mARSession = ARKit::Session::create( fmt );
    mARSession->run();
}

void BasicSampleApp::touchesBegan( TouchEvent event )
{
    // Add a point 50cm in front of camera
    mARSession->addAnchorRelativeToCamera( vec3(0.0f, 0.0f, -0.5f) );
}

void BasicSampleApp::update()
{
}

void BasicSampleApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    gl::color( 1.0f, 1.0f, 1.0f, 1.0f );
    mARSession->drawRGBCaptureTexture( getWindowBounds() );
    
    gl::ScopedMatrices matScp;
    gl::setViewMatrix( mARSession->mViewMatrix );
    gl::setProjectionMatrix( mARSession->mProjectionMatrix );
    
    gl::ScopedGlslProg glslProg( gl::getStockShader( gl::ShaderDef().color() ));
    gl::ScopedColor colScp;
    gl::color( 1.0f, 1.0f, 1.0f );
    
    for (const auto& a : mARSession->getAnchors())
    {
        gl::ScopedMatrices matScp;
        gl::setModelMatrix( a.mTransform );
        
        gl::drawStrokedCube( vec3(0.0f), vec3(0.02f) );
    }
    
    for (const auto& a : mARSession->getPlaneAnchors())
    {
        gl::ScopedMatrices matScp;
        gl::setModelMatrix( a.mTransform );
        gl::translate( a.mCenter );
        gl::rotate( (float)M_PI * 0.5f, vec3(1,0,0) ); // Make it parallel with the ground
        const float xRad = a.mExtent.x * 0.5f;
        const float zRad = a.mExtent.z * 0.5f;
        gl::color( 0.0f, 0.6f, 0.9f, 0.2f );
        gl::drawSolidRect( Rectf( -xRad,-zRad, xRad, zRad ));
    }
}

CINDER_APP( BasicSampleApp, RendererGl )

