//
//  ARSessionImpl.h
//  ARKitTest
//
//  Created by Felix Faire on 11/08/2017.
//

#ifndef ARSessionImpl_h
#define ARSessionImpl_h

#include "cinder/gl/gl.h"
#include "ARAnchorTypes.h"

using namespace ci;

namespace ARKit {

enum TrackingConfiguration
{
    OrientationTracking,
    WorldTracking,
    WorldTrackingWithHorizontalPlaneDetection,
    WorldTrackingWithVerticalPlaneDetection,
};


class SessionImpl
{
public:
    
    SessionImpl();
    ~SessionImpl();
    
    void runConfiguration( TrackingConfiguration config );
    void pause();
    
    /**  Adds an anchor point to the ARSession relative to the current world orientation.
         Returns the AnchorID of the anchor to reference later on.
    */
    const AnchorID addAnchorRelativeToWorld( vec3 position );
    
    /**  Adds an anchor point to the ARSession relative from the current camera position
         and orienation. Returns the AnchorID of the anchor to reference later on.
    */
    const AnchorID addAnchorRelativeToCamera( vec3 offset );

    /**  Returns true if the interface / app is running in portrait orientation.
    */
    bool isInterfaceInPortraitOrientation() const;
    
    
    std::vector<Anchor>       mAnchors;
    std::vector<PlaneAnchor>  mPlaneAnchors;
    
    bool                   mRGBCaptureEnabled = true;
    Channel8u              mFrameYChannel;
    Channel8u              mFrameCbChannel;
    Channel8u              mFrameCrChannel;

    mat4                   mViewMatrix;
    mat4                   mProjectionMatrix;
    
    float                  mAmbientLightIntensity;
    float                  mAmbientColorTemperature;

    bool                   mIsRunning = false;
    
    vec2                   mCameraSize = vec2( 1.0f );
};

} // namespace ARKit

#endif /* ARSessionImpl_h */
