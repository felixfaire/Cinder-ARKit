#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderARKit.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicViewProjectionApp : public App {
  public:
	void setup() override;
	void touchesBegan( TouchEvent event ) override;
	void update() override;
	void draw() override;
	
	CinderARKit mARKit;
};

void BasicViewProjectionApp::setup()
{
    mARKit.setViewSize( getWindowSize() );
}

void BasicViewProjectionApp::touchesBegan( TouchEvent event )
{
    // Add a point 50cm in front of camera
    mARKit.addAnchorRelativeToCamera( vec3(0.0f, 0.0f, -0.5f) );
}

void BasicViewProjectionApp::update()
{
}

void BasicViewProjectionApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	//gl::draw( mARKit.getFrameTexture(), getWindowBounds() );
	
    gl::ScopedMatrices matScp;
    gl::setViewMatrix( mARKit.mViewMatrix );
    gl::setProjectionMatrix( mARKit.mProjectionMatrix );
    
    gl::ScopedGlslProg glslProg( gl::getStockShader( gl::ShaderDef().color() ));
    gl::color( 1.0f, 1.0f, 1.0f );
    
    for (const auto& a : mARKit.mAnchorMap)
    {
        gl::ScopedMatrices matScp;
        gl::setModelMatrix( a.second.mTransform );
        
        gl::drawCube( vec3(0.0f), vec3(0.02f) );
    }
    
//    // Translate 1m in front of camera
//    gl::translate( vec3(0.0f, 0.0f, -1.0f ));
//
//    // Draw an object
//    for (float t = 0.0f; t < 1.0f; t += 0.05f)
//    {
//        gl::ScopedMatrices rotScp;
//        gl::rotate( t, sin(t * 2.0f), cos(t * 3.0f), sin(t * 4.0) );
//        const float size = 0.1f + t * 2.0f;
//        gl::drawStrokedCube( vec3(0.0f), vec3(size) );
//    }
}

CINDER_APP( BasicViewProjectionApp, RendererGl )
