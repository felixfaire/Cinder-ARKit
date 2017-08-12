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
    mARSession = ARKit::Session::create();
    mARSession->setViewSize( getWindowSize() );
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
    
    gl::ScopedMatrices matScp;
    gl::setViewMatrix( mARSession->mViewMatrix );
    gl::setProjectionMatrix( mARSession->mProjectionMatrix );
    
    gl::ScopedGlslProg glslProg( gl::getStockShader( gl::ShaderDef().color() ));
    gl::color( 1.0f, 1.0f, 1.0f );
    
    for (const auto& a : mARSession->mAnchorMap)
    {
        gl::ScopedMatrices matScp;
        gl::setModelMatrix( a.second.mTransform );
        
        gl::drawCube( vec3(0.0f), vec3(0.02f) );
    }
}

CINDER_APP( BasicSampleApp, RendererGl )

