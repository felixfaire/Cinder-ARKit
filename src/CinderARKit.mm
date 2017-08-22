//
//  CinderARKit.mm
//  CinderARKit
//
//  Created by Felix Faire on 11/08/2017.
//


#import <Foundation/Foundation.h>
#import <ARKit/ARKit.h>

@interface ARKitSessionImpl : NSObject<ARSessionDelegate>
{
@public
    ARSession*       mARSession;
}
- (void)session:(ARSession*)session didUpdateFrame:(ARFrame*)frame;
- (void)session:(ARSession*)session didAddAnchors:(NSArray<ARAnchor*>*)anchors;
- (void)session:(ARSession*)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors;
- (void)session:(ARSession*)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors;

- (void)addAnchorWithxOffset:(NSNumber*)x yOffset:(NSNumber*)y zOffset:(NSNumber *)z;

@end

#import "CinderARKit.h"
#import "CinderARKitUtils.h"

#include "cinder/gl/gl.h"

using namespace ARKit;

// Global instances of the session to bridge C++ and Obj C
static ARKitSessionImpl* ciARKitSessionImpl = nil;
static Session* ciARKitSession = nullptr;

SessionRef Session::create( Session::Format format )
{
    return std::make_shared<Session>( format );
}

Session::Session( Session::Format format )
    : mFormat( format )
{
    // You already have an ARSession instance running
    DBG_ASSERT(ciARKitSession == nullptr)
    
    ciARKitSession = this;
    
    mYCbCrToRGBProg = getYCbCrToRBGGlslProgram();

    if (ciARKitSessionImpl == nil)
    {
        ciARKitSessionImpl = [[ARKitSessionImpl alloc] init];
        
        DBG_ASSERT(ciARKitSessionImpl);
    }
}

Session::~Session()
{
    DBG_ASSERT(ciARKitSession == this);
    ciARKitSession = nullptr;
}

void Session::run()
{
    runConfiguration( mFormat.mConfiguration );
}

void Session::runConfiguration( TrackingConfiguration config )
{
    mFormat.mConfiguration = config;
    ARConfiguration* configuration = getNativeARConfiguration( config );
    [ciARKitSessionImpl->mARSession runWithConfiguration:configuration];
}

void Session::pause()
{
    mIsRunning = false;
    [ciARKitSessionImpl->mARSession pause];
}

void Session::addAnchorRelativeToCamera( vec3 offset )
{
    [ciARKitSessionImpl addAnchorWithxOffset:@(offset.x) yOffset:@(offset.y) zOffset:@(offset.z)];
}

void Session::drawRGBCaptureTexture( Area area )
{
    if (!mIsRunning)
        return;
    
    const auto texY  = gl::Texture2d::create( mFrameYChannel );
    const auto texCr = gl::Texture2d::create( mFrameCrChannel );
    const auto texCb = gl::Texture2d::create( mFrameCbChannel );
    
    auto cameraRect = Rectf( vec2( 0.0f ), mCameraSize );
    bool rotate = false;
    
    gl::ScopedMatrices matScp;
    
    if (UIInterfaceOrientationIsPortrait( [[UIApplication sharedApplication] statusBarOrientation] ))
    {
        cameraRect = Rectf( vec2( 0.0f ), vec2( mCameraSize.y, mCameraSize.x ));
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
    gl::drawSolidRect( cameraRect.getCenteredFit( area, true ));
}

std::vector<Anchor> Session::getAnchors()
{
    std::vector<Anchor> anchors;
    
    for (const auto& a : mAnchors)
        anchors.push_back( a.second );
    
    return anchors;
}

std::vector<PlaneAnchor> Session::getPlaneAnchors()
{
    std::vector<PlaneAnchor> planeAnchors;
    
    for (const auto& a : mPlaneAnchors)
        planeAnchors.push_back( a.second );
    
    return planeAnchors;
}





@implementation ARKitSessionImpl

- (id) init
{
    self = [super init];
    
    mARSession = [ARSession new];
    mARSession.delegate = self;
    
    return self;
}

- (void)session:(ARSession*)session didUpdateFrame:(ARFrame*)frame
{
    ciARKitSession->mIsRunning = true;
    
    auto orientation = [[UIApplication sharedApplication] statusBarOrientation];
    
    // Update view matrix
    ciARKitSession->mViewMatrix = toMat4([frame.camera viewMatrixForOrientation:orientation]);
    
    // Update projection matrix
    auto viewBounds = [[UIScreen mainScreen] bounds];
    CGSize viewportSize = viewBounds.size;
    ciARKitSession->mProjectionMatrix = toMat4([frame.camera projectionMatrixForOrientation:orientation
                                                                       viewportSize:viewportSize
                                                                             zNear:0.001
                                                                              zFar:1000]);
    // Update light estimate
    ciARKitSession->mAmbientLightIntensity = (float)[[frame lightEstimate] ambientIntensity] / 2000.0f;
    ciARKitSession->mAmbientColorTemperature = (float)[[frame lightEstimate] ambientColorTemperature];
    
    // Capture pixel YCbCr
    CVPixelBufferRef pixelBuffer = frame.capturedImage;
    ciARKitSession->mFrameYChannel  = getChannelForCVPixelBuffer( pixelBuffer, 0 );
    ciARKitSession->mFrameCbChannel = getChannelForCVPixelBuffer( pixelBuffer, 1, 2, 0 );
    ciARKitSession->mFrameCrChannel = getChannelForCVPixelBuffer( pixelBuffer, 1, 2, 1 );
    
    ciARKitSession->mCameraSize = vec2( (float)CVPixelBufferGetWidth( pixelBuffer ), (float)CVPixelBufferGetHeight( pixelBuffer ));
}

// ===== ANCHOR HANDLING ===========================================================================================
- (void)session:(ARSession *)session didAddAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (ARAnchor* anchor in anchors)
    {
        if ([anchor isKindOfClass:[ARPlaneAnchor class]])
        {
            ARPlaneAnchor* pa = (ARPlaneAnchor*)anchor;
            ciARKitSession->mPlaneAnchors[getUidStringFromAnchor( pa )] = PlaneAnchor( toMat4( pa.transform ), toVec3( pa.center ), toVec3( pa.extent ));
        }
        else
        {
            ciARKitSession->mAnchors[getUidStringFromAnchor( anchor )] = Anchor( toMat4( anchor.transform ));
        }
    }
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (ARAnchor* anchor in anchors)
    {
        if ( [anchor isKindOfClass:[ARPlaneAnchor class]] )
        {
            ARPlaneAnchor* pa = (ARPlaneAnchor*)anchor;
            ciARKitSession->mPlaneAnchors[getUidStringFromAnchor( pa )] = PlaneAnchor( toMat4( pa.transform ), toVec3( pa.center ), toVec3( pa.extent ));
        }
        else
        {
            ciARKitSession->mAnchors[getUidStringFromAnchor( anchor )] = Anchor( toMat4( anchor.transform ));
        }
    }
}

- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (ARAnchor* anchor in anchors)
        ciARKitSession->mAnchors.erase( getUidStringFromAnchor( anchor ));
}

- (void)addAnchorWithxOffset:(NSNumber*)x yOffset:(NSNumber*)y zOffset:(NSNumber*)z;
{
    ARFrame* currentFrame = [mARSession currentFrame];
    
    // Create anchor using the camera's current position
    if (currentFrame)
    {
        // Create a transform relative to the camera view
        matrix_float4x4 translation = matrix_identity_float4x4;
        translation.columns[3].x = [x floatValue];
        translation.columns[3].z = [y floatValue];
        translation.columns[3].z = [z floatValue];
        matrix_float4x4 transform = matrix_multiply(currentFrame.camera.transform, translation);
        ARAnchor *anchor = [[ARAnchor alloc] initWithTransform:transform];
        [mARSession addAnchor:anchor];
    }
}



@end

