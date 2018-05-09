#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderARKit.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ImageTrackingApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
 
     ARKit::Session     mARSession;
    
     gl::Texture2dRef   mTexture;
};

void ImageTrackingApp::setup()
{
    // Tracked images are stored in Images.xcassets / AR Resources
    // You must set the image size of the asset in real world units.
    // The image in this demo is set to 12cm x 12cm which is roughly the size of the
    // image when you open it in preview.
    mARSession.runConfiguration(ARKit::TrackingConfiguration::ReferenceImageTrackingConfiguration);
    
    // This texture is just used for visualisation
    mTexture = gl::Texture2d::create(loadImage(loadAsset("kandinsky.png")));
}

void ImageTrackingApp::update()
{
}

void ImageTrackingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
 
    gl::color( 1.0f, 1.0f, 1.0f, 1.0f );
    mARSession.drawRGBCaptureTexture( getWindowBounds() );
    
    gl::ScopedMatrices matScp;
    gl::setViewMatrix( mARSession.getViewMatrix() );
    gl::setProjectionMatrix( mARSession.getProjectionMatrix() );
    
    gl::ScopedGlslProg glslProg( gl::getStockShader( gl::ShaderDef().color().texture() ));
    gl::ScopedTextureBind texScp( mTexture );
    gl::ScopedColor colScp;
    gl::color( 1.0f, 1.0f, 1.0f );
    
    for (const auto& a : mARSession.getImageAnchors())
    {
        gl::ScopedMatrices matScp;
        gl::setModelMatrix( a.mTransform );
        
        gl::color( 1.0f, 1.0f, 1.0f, 1.0f );
        
        gl::rotate( (float)M_PI * 0.5f, vec3(1,0,0) ); // Make it parallel with the ground
        const float xRad = a.mPhysicalSize.x * 0.5f;
        const float yRad = a.mPhysicalSize.y * 0.5f;

        // Overlay tracked image
        for (int i = 0; i < 10; ++i)
        {
            if (i < (int)(getElapsedSeconds() * 5.0f) % 10)
            {
                gl::translate( 0.0f, 0.0f, -0.01f );
                gl::drawSolidRect( Rectf( -xRad,-yRad, xRad, yRad ));
            }
        }

        /* a.mImageName will allow to decide what AR content to show, attached to this anchor*/
    }
}

CINDER_APP( ImageTrackingApp, RendererGl )
