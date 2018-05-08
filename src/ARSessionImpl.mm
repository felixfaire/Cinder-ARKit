//
//  ARSessionImpl.mm
//  CinderARKit
//
//  Created by Felix Faire on 11/08/2017.
//


#import <Foundation/Foundation.h>
#import <ARKit/ARKit.h>

@interface AppleARKitSessionImpl : NSObject<ARSessionDelegate>
{
@public
    ARSession*       mARSession;
}
- (void)session:(ARSession*)session didUpdateFrame:(ARFrame*)frame;
- (void)session:(ARSession*)session didAddAnchors:(NSArray<ARAnchor*>*)anchors;
- (void)session:(ARSession*)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors;
- (void)session:(ARSession*)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors;

- (NSUUID*)addAnchorRelativeToWorldWithxPos:(NSNumber*)x yPos:(NSNumber*)y zPos:(NSNumber*)z;
- (NSUUID*)addAnchorRelativeToCameraWithxOffset:(NSNumber*)x yOffset:(NSNumber*)y zOffset:(NSNumber*)z;

@end

#import "ARSessionImpl.h"
#import "CinderARKitUtils.h"

#include "cinder/gl/gl.h"


// Global instances of the AR session to bridge C++ and Obj C
static AppleARKitSessionImpl* appleARKitSession = nil;   // Apple ARKit API
static ARKit::SessionImpl*    ciARKitSession = nullptr;       // Cinder ARKit Bridge


using namespace ARKit;

// Cinder ARKit Session implementation
SessionImpl::SessionImpl()
{
    // You already have an ARSession instance running
    DBG_ASSERT(ciARKitSession == nullptr)
    
    ciARKitSession = this;

    if (appleARKitSession == nil)
    {
        appleARKitSession = [[AppleARKitSessionImpl alloc] init];
        
        DBG_ASSERT(appleARKitSession);
    }
}

SessionImpl::~SessionImpl()
{
    DBG_ASSERT(ciARKitSession == this);
    ciARKitSession = nullptr;
}

void SessionImpl::runConfiguration( TrackingConfiguration config )
{
    ARConfiguration* configuration = getNativeARConfiguration( config );
    [appleARKitSession->mARSession runWithConfiguration:configuration];
}

void SessionImpl::pause()
{
    mIsRunning = false;
    [appleARKitSession->mARSession pause];
}

const AnchorID SessionImpl::addAnchorRelativeToCamera( vec3 offset )
{
    NSUUID* uid = [appleARKitSession addAnchorRelativeToCameraWithxOffset:@(offset.x) yOffset:@(offset.y) zOffset:@(offset.z)];
    return getUidStringFromUUID(uid);
}

bool SessionImpl::isInterfaceInPortraitOrientation() const
{
    return UIInterfaceOrientationIsPortrait( [[UIApplication sharedApplication] statusBarOrientation] );
}




// Apple ARKit API Bridge implementation

@implementation AppleARKitSessionImpl

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
        const auto uid = getUidStringFromAnchor( anchor );
        
        if ([anchor isKindOfClass:[ARPlaneAnchor class]])
        {
            ARPlaneAnchor* pa = (ARPlaneAnchor*)anchor;
            updateOrAddAnchor(ciARKitSession->mPlaneAnchors, PlaneAnchor( uid, toMat4( pa.transform ), toVec3( pa.center ), toVec3( pa.extent )));
        }
        else
        {
            updateOrAddAnchor(ciARKitSession->mAnchors, Anchor( uid, toMat4( anchor.transform )));
        }
    }
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (ARAnchor* anchor in anchors)
    {
        const auto uid = getUidStringFromAnchor( anchor );
        
        if ( [anchor isKindOfClass:[ARPlaneAnchor class]] )
        {
            ARPlaneAnchor* pa = (ARPlaneAnchor*)anchor;
            updateOrAddAnchor(ciARKitSession->mPlaneAnchors, PlaneAnchor( uid, toMat4( pa.transform ), toVec3( pa.center ), toVec3( pa.extent )));
        }
        else
        {
            updateOrAddAnchor(ciARKitSession->mAnchors, Anchor( uid, toMat4( anchor.transform )));
        }
    }
}

- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (ARAnchor* anchor in anchors)
    {
        if ( [anchor isKindOfClass:[ARPlaneAnchor class]] )
            removeAnchorWithID( ciARKitSession->mPlaneAnchors, getUidStringFromAnchor( anchor ));
        else
            removeAnchorWithID( ciARKitSession->mAnchors, getUidStringFromAnchor( anchor ));
    }
}

- (NSUUID*)addAnchorRelativeToWorldWithxPos:(NSNumber*)x yPos:(NSNumber*)y zPos:(NSNumber*)z;
{
    ARFrame* currentFrame = [mARSession currentFrame];
    NSUUID* anchorID;
    
    // Create anchor using the camera's current position
    if (currentFrame)
    {
        // Create a transform relative to the camera view
        matrix_float4x4 translation = matrix_identity_float4x4;
        translation.columns[3].x = [x floatValue];
        translation.columns[3].z = [y floatValue];
        translation.columns[3].z = [z floatValue];
        ARAnchor *anchor = [[ARAnchor alloc] initWithTransform:translation];
        anchorID = anchor.identifier;
        [mARSession addAnchor:anchor];
    }
    else
    {
        CI_ASSERT_MSG(false, "No current ARFrame, this will return an invalid anchor");
    }
    
    return anchorID;
}

- (NSUUID*)addAnchorRelativeToCameraWithxOffset:(NSNumber*)x yOffset:(NSNumber*)y zOffset:(NSNumber*)z;
{
    ARFrame* currentFrame = [mARSession currentFrame];
    NSUUID* anchorID;
    
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
        anchorID = anchor.identifier;
        [mARSession addAnchor:anchor];
    }
    else
    {
        CI_ASSERT_MSG(false, "No current ARFrame, this will return an invalid anchor");
    }
    
    return anchorID;
}



@end

