//
//  CinderARKit.mm
//  CinderARKit
//
//  Created by Felix Faire on 11/08/2017.
//

#import <Foundation/Foundation.h>
#import <ARKit/ARKit.h>

#define AR_ASSERT(expression) if (! (expression)) ::kill (0, SIGTRAP);

@interface CinderARKitImpl : NSObject<ARSessionDelegate>
{
    ARSession* mARSession;
}
- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame;
- (void)addAnchorWithxOffset:(NSNumber *)x yOffset:(NSNumber *)y zOffset:(NSNumber *)z;

@end


#import "CinderARKit.h"
#import "CinderARKitUtils.h"

// Global instances of the session to bridge C++ and Obj C
static CinderARKitImpl* cinderArKitImpl = nil;
static CinderARKit* cinderArKit = nullptr;

CinderARKit::CinderARKit()
{
    AR_ASSERT(cinderArKit == nullptr)
    
    cinderArKit = this;

    if (cinderArKitImpl == nil)
    {
        cinderArKitImpl = [[CinderARKitImpl alloc] init];
        
        AR_ASSERT(cinderArKitImpl);
    }
}

CinderARKit::~CinderARKit()
{
    AR_ASSERT(cinderArKit == this);
    cinderArKit = nullptr;
}

void CinderARKit::addAnchorRelativeToCamera( vec3 offset )
{
    std::cout << "Adding anchor point" << std::endl;
    [cinderArKitImpl addAnchorWithxOffset:@(offset.x) yOffset:@(offset.y) zOffset:@(offset.z)];
}



@implementation CinderARKitImpl

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
    cinderArKit->mViewMatrix = fromMtl(matrix_invert(frame.camera.transform));
    
    // Update projection matrix
    vec2 size = cinderArKit->mViewSize;
    CGSize viewportSize = CGSizeMake(size.y, size.x);
    cinderArKit->mProjectionMatrix = fromMtl([frame.camera projectionMatrixForOrientation:UIInterfaceOrientationLandscapeRight
                                                                       viewportSize:viewportSize
                                                                             zNear:0.001
                                                                              zFar:1000]);
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

