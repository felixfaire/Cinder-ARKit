//
//  CinderARKit.cpp
//  BasicSample
//
//  Created by Felix Faire on 08/05/2018.
//

#include "CinderARKit.h"

using namespace ARKit;

Session::Session()
{
    mYCbCrToRGBProg = createCameraRGBProg();
}

void Session::runConfiguration( TrackingConfiguration config )      { mSessionImpl.runConfiguration( config ); }
void Session::pause()                                               { mSessionImpl.pause(); }
const AnchorID Session::addAnchorRelativeToWorld( vec3 position )   { return mSessionImpl.addAnchorRelativeToWorld( position ); }
const AnchorID Session::addAnchorRelativeToCamera( vec3 offset )    { return mSessionImpl.addAnchorRelativeToCamera( offset ); }
gl::Texture2dRef Session::getFrameLumaTexture() const               { return gl::Texture2d::create( mSessionImpl.mFrameYChannel ); }
float Session::getAmbientLightIntensity() const                     { return mSessionImpl.mAmbientLightIntensity; }
float Session::getAmbientColorTemperature() const                   { return mSessionImpl.mAmbientColorTemperature; }
const std::vector<Anchor>& Session::getAnchors() const              { return mSessionImpl.mAnchors; }
const std::vector<PlaneAnchor>& Session::getPlaneAnchors() const    { return mSessionImpl.mPlaneAnchors; }
const std::vector<ImageAnchor>& Session::getImageAnchors() const    { return mSessionImpl.mImageAnchors; }
void Session::setRGBCaptureEnabled( bool captureEnabled )           { mSessionImpl.mRGBCaptureEnabled = captureEnabled; }
const mat4 Session::getViewMatrix() const                           { return mSessionImpl.mViewMatrix; }
const mat4 Session::getProjectionMatrix() const                     { return mSessionImpl.mProjectionMatrix; }

std::shared_ptr<Anchor> Session::findAnchorWithID( AnchorID anchorID ) const
{
    const auto foundAnchor = std::find_if( mSessionImpl.mAnchors.begin(), mSessionImpl.mAnchors.end(), [anchorID](const Anchor& a) { return (a.mUid == anchorID); });
    
    if (foundAnchor != mSessionImpl.mAnchors.end())
        return std::make_shared<Anchor>( *foundAnchor );
    
    return nullptr;
}

void Session::drawRGBCaptureTexture( Area area ) const
{
    if (!mSessionImpl.mIsRunning)
        return;
    
    const auto texY  = gl::Texture2d::create( mSessionImpl.mFrameYChannel );
    const auto texCr = gl::Texture2d::create( mSessionImpl.mFrameCrChannel );
    const auto texCb = gl::Texture2d::create( mSessionImpl.mFrameCbChannel );
    
    auto cameraRect = Rectf( vec2( 0.0f ), mSessionImpl.mCameraSize );
    bool rotate = false;
    
    gl::ScopedMatrices matScp;
    
    if (mSessionImpl.isInterfaceInPortraitOrientation())
    {
        cameraRect = Rectf( vec2( 0.0f ), vec2( mSessionImpl.mCameraSize.y, mSessionImpl.mCameraSize.x ));
        rotate = true;
    }
    
    gl::ScopedGlslProg glslProg( mYCbCrToRGBProg );
    gl::ScopedTextureBind yScp( texY, 0 );
    gl::ScopedTextureBind cbScp( texCb, 1 );
    gl::ScopedTextureBind crScp( texCr, 2 );
    mYCbCrToRGBProg->uniform( "u_YTex", 0 );
    mYCbCrToRGBProg->uniform( "u_CbTex", 1 );
    mYCbCrToRGBProg->uniform( "u_CrTex", 2 );
    mYCbCrToRGBProg->uniform( "u_Rotate", rotate );
    gl::drawSolidRect( cameraRect.getCenteredFill( area, true ));
}

gl::GlslProgRef Session::createCameraRGBProg()
{
    return gl::GlslProg::create( gl::GlslProg::Format()
            .vertex( CI_GLSL(100, precision mediump float;

            uniform mat4 ciModelViewProjection;
            uniform bool u_Rotate;

            attribute vec4 ciPosition;
            attribute vec2 ciTexCoord0;

            varying vec2 v_TexCoord;
    
            const float rotTheta = -3.1415926536 * 0.5;
            const mat2 rot = mat2(cos(rotTheta), -sin(rotTheta), sin(rotTheta), cos(rotTheta));

            void main()
            {
                v_TexCoord = ciTexCoord0;
                
                if (u_Rotate)
                {
                    v_TexCoord -= vec2( 0.5 );
                    v_TexCoord = rot * v_TexCoord;
                    v_TexCoord += vec2( 0.5 );
                }
                
                gl_Position = ciModelViewProjection * ciPosition;
            }))
            .fragment( CI_GLSL(100, precision mediump float;
    
            uniform sampler2D u_YTex;
            uniform sampler2D u_CbTex;
            uniform sampler2D u_CrTex;
    
            varying vec2 v_TexCoord;

            void main()
            {
                vec3 rgb = vec3( 0.0 );
                
                float y  = texture2D( u_YTex,  v_TexCoord ).r;
                float cb = texture2D( u_CbTex, v_TexCoord ).r - 0.5;
                float cr = texture2D( u_CrTex, v_TexCoord ).r - 0.5;

                rgb.x = y + 1.402 * cr;
                rgb.y = y -0.344 * cb - 0.714 * cr;
                rgb.z = y + 1.772 * cb;
                
                gl_FragColor = vec4( rgb, 1.0 );
            })));
}

