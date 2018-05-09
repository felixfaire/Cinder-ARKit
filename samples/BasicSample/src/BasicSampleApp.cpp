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
    
    ARKit::Session mARSession;
};

void BasicSampleApp::setup()
{
#if 1

    mARSession.runConfiguration(ARKit::TrackingConfiguration::ReferenceImageTrackingConfiguration);

#else
    const auto config = ARKit::TrackingConfiguration::WorldTrackingWithHorizontalPlaneDetection;
    mARSession.runConfiguration( config );
#endif
}

void BasicSampleApp::touchesBegan( TouchEvent event )
{
    // Add a point 50cm in front of camera
    mARSession.addAnchorRelativeToCamera( vec3(0.0f, 0.0f, -0.5f) );
}

void BasicSampleApp::update()
{
}

void BasicSampleApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    gl::color( 1.0f, 1.0f, 1.0f, 1.0f );
    mARSession.drawRGBCaptureTexture( getWindowBounds() );
    
    gl::ScopedMatrices matScp;
    gl::setViewMatrix( mARSession.getViewMatrix() );
    gl::setProjectionMatrix( mARSession.getProjectionMatrix() );
    
    gl::ScopedGlslProg glslProg( gl::getStockShader( gl::ShaderDef().color() ));
    gl::ScopedColor colScp;
    gl::color( 1.0f, 1.0f, 1.0f );
    
    for (const auto& a : mARSession.getAnchors())
    {
        gl::ScopedMatrices matScp;
        gl::setModelMatrix( a.mTransform );
        gl::drawStrokedCube( vec3(0.0f), vec3(0.02f) );
    }
    
    for (const auto& a : mARSession.getPlaneAnchors())
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
    
    for (const auto& a : mARSession.getImageAnchors())
    {
        gl::ScopedMatrices matScp;
        gl::setModelMatrix( a.mTransform );
        gl::translate( vec2(a.mPhysicalSize.x * 0.5f, a.mPhysicalSize.y * 0.5f) );
        gl::rotate( (float)M_PI * 0.5f, vec3(1,0,0) ); // Make it parallel with the ground
        const float xRad = a.mPhysicalSize.x * 0.5f;
        const float yRad = a.mPhysicalSize.y * 0.5f;
        gl::color( 0.0f, 0.6f, 0.9f, 0.2f );
        gl::drawSolidRect( Rectf( -xRad,-yRad, xRad, yRad ));
        
        /* a.mImageName will allow to decide what AR content to show, attached to this anchor*/
    }
}

CINDER_APP( BasicSampleApp, RendererGl )

