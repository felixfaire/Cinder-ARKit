//
//  CinderARKit.mm
//  CinderARKit
//
//  Created by Felix Faire on 11/08/2017.
//

#if CI_DBG
  #define DBG_ASSERT(expression) if (! (expression)) ::kill (0, SIGTRAP);
#else
  #define DBG_ASSERT(expression)
#endif

#import <Foundation/Foundation.h>
#import <ARKit/ARKit.h>

@interface ARKitSessionImpl : NSObject<ARSessionDelegate>
{
    ARSession*       mARSession;
    
    int32_t          mSurfaceChannelOrderCode;
    int32_t          mExposedFrameBytesPerRow;
    int32_t          mExposedFrameHeight;
    int32_t          mExposedFrameWidth;
}
- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame;
- (void)session:(ARSession *)session didAddAnchors:(NSArray<ARAnchor*>*)anchors;
- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors;
- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors;

- (void)addAnchorWithxOffset:(NSNumber *)x yOffset:(NSNumber *)y zOffset:(NSNumber *)z;

@end

#import "CinderARKit.h"
#import "CinderARKitUtils.h"

// Global instances of the session to bridge C++ and Obj C
static ARKitSessionImpl* ciARKitSessionImpl = nil;
static ARKit::Session* ciARKitSession = nullptr;

ARKit::Session::Session()
{
    DBG_ASSERT(ciARKitSession == nullptr)
    
    ciARKitSession = this;

    if (ciARKitSessionImpl == nil)
    {
        ciARKitSessionImpl = [[ARKitSessionImpl alloc] init];
        
        DBG_ASSERT(ciARKitSessionImpl);
    }
}

ARKit::Session::~Session()
{
    DBG_ASSERT(ciARKitSession == this);
    ciARKitSession = nullptr;
}

void ARKit::Session::addAnchorRelativeToCamera( vec3 offset )
{
    std::cout << "Adding anchor point" << std::endl;
    [ciARKitSessionImpl addAnchorWithxOffset:@(offset.x) yOffset:@(offset.y) zOffset:@(offset.z)];
}



@implementation ARKitSessionImpl

- (id) init
{
    self = [super init];
    
    mARSession = [ARSession new];
    mARSession.delegate = self;
    
    ARWorldTrackingConfiguration *configuration = [ARWorldTrackingConfiguration new];
    configuration.planeDetection = ARPlaneDetectionHorizontal;
    [mARSession runWithConfiguration:configuration];
    
    return self;
}

- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame
{
    // Update view matrix
    ciARKitSession->mViewMatrix = fromMtl(matrix_invert(frame.camera.transform));
    
    // Update projection matrix
    vec2 size = ciARKitSession->mViewSize;
    CGSize viewportSize = CGSizeMake(size.y, size.x);
    ciARKitSession->mProjectionMatrix = fromMtl([frame.camera projectionMatrixForOrientation:UIInterfaceOrientationLandscapeRight
                                                                       viewportSize:viewportSize
                                                                             zNear:0.001
                                                                              zFar:1000]);

    // Update capture image
    CVPixelBufferRef pixelBuffer = frame.capturedImage;
    uint8_t* data = (uint8_t *)CVPixelBufferGetBaseAddress( pixelBuffer );
    mExposedFrameBytesPerRow = (int32_t)CVPixelBufferGetBytesPerRow( pixelBuffer );
    mExposedFrameWidth = (int32_t)CVPixelBufferGetWidth( pixelBuffer );
    mExposedFrameHeight = (int32_t)CVPixelBufferGetHeight( pixelBuffer );

    ciARKitSession->mCurrentFrame = cinder::Surface8u( data, mExposedFrameWidth, mExposedFrameHeight, mExposedFrameBytesPerRow, cinder::SurfaceChannelOrder::BGRA );
}

// ===== ANCHOR HANDLING ===========================================================================================
- (void)session:(ARSession *)session didAddAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (ARAnchor* anchor in anchors)
        ciARKitSession->mAnchorMap[getUidStringFromAnchor( anchor )] = { fromMtl( anchor.transform ) };
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (ARAnchor* anchor in anchors)
        ciARKitSession->mAnchorMap[getUidStringFromAnchor( anchor )] = { fromMtl( anchor.transform ) };
}

- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (ARAnchor* anchor in anchors)
        ciARKitSession->mAnchorMap.erase(getUidStringFromAnchor( anchor ));
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

