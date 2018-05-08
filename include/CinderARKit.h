//
//  CinderARKit.h
//  ARKitTest
//
//  Created by Felix Faire on 11/08/2017.
//

#ifndef CinderARKit_h
#define CinderARKit_h

#include "cinder/gl/gl.h"

using namespace ci;

/**
     CinderARKit is a WIP bridge to expose the native
     AR functionality of iOS 11 in Cinder.
*/
namespace ARKit {

class Session;
typedef std::shared_ptr<Session> SessionRef;

typedef std::string AnchorID;

/**  An anchor point that will be tracked by ARKit*/
class Anchor
{
public:
    Anchor() {}
    Anchor( std::string uid, mat4 transform )
        : mUid( uid ),
          mTransform( transform ) {}
    
    AnchorID    mUid;
    mat4        mTransform;
};

/**  An anchor point that includes plane position and extents*/
class PlaneAnchor
{
public:
    PlaneAnchor() {}
    PlaneAnchor( std::string uid, mat4 transform, vec3 center, vec3 extent )
        : mUid( uid ),
          mTransform( transform ),
          mCenter( center ),
          mExtent( extent ) {}
    
    AnchorID        mUid;
    mat4            mTransform;
    vec3            mCenter;
    vec3            mExtent;
};

class Session
{
public:
    
    enum TrackingConfiguration
    {
        OrientationTracking,
        WorldTracking,
        WorldTrackingWithHorizontalPlaneDetection,
        WorldTrackingWithVerticalPlaneDetection,
    };

    /**  Format contains the options for the session when it is created
    */
    class Format
    {
    public:
        Format(){}
        Format& configuration( TrackingConfiguration config ) { mConfiguration = config; return *this; }
        Format& rgbCaptureEnabled( bool rgbEnabled ) { mRGBCaptureEnabled = rgbEnabled; return *this; }
        
        TrackingConfiguration mConfiguration = { WorldTrackingWithHorizontalPlaneDetection };
        bool                  mRGBCaptureEnabled = true;
    };
    
    static SessionRef create( Format format = Format() );
    
    Session( Format format = Format() );
    ~Session();
    
    void run();
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

    /**  Returns the luma texture for the current frame capture
    */
    gl::Texture2dRef getFrameLumaTexture() const   { return gl::Texture2d::create( mFrameYChannel ); }
    
    /**  Draws the converted rgb capture to the desired area
    */
    void drawRGBCaptureTexture( Area area ) const;
    
    /**  Returns the estimated light intensity for the scene
         0.0 (very dark) 1.0 (very bright)
    */
    float getAmbientLightIntensity() const        { return mAmbientLightIntensity; }
    
    /**  Returns the estimated light color temperature for the scene in Kelvin
         (6500 is pure white)
    */
    float getAmbientColorTemperature() const     { return mAmbientColorTemperature; }
    
    /**  Get the list of anchor transforms (points with orientations)
    */
    const std::vector<Anchor>& getAnchors() const { return mAnchors; }
    
    /**  Get the full list of plane anchor objects
    */
    const std::vector<PlaneAnchor>& getPlaneAnchors() const { return mPlaneAnchors; }
    
    /**  Finds the anchor with a certain ID if it exists. Returns an empty
         std::shared_ptr<Anchor> if it doesn't exist.
    */
    std::shared_ptr<Anchor> findAnchorWithID( AnchorID anchorID ) const;
    
    
    Channel8u              mFrameYChannel;
    Channel8u              mFrameCbChannel;
    Channel8u              mFrameCrChannel;

    mat4                   mViewMatrix;
    mat4                   mProjectionMatrix;
    
    // Currently members are publically exposed to Obj C Implementation
    // You shouldnt need to manipulate the Anchor arrays directly.
    
    std::vector<Anchor>       mAnchors;
    std::vector<PlaneAnchor>  mPlaneAnchors;
    
    float                  mAmbientLightIntensity;
    float                  mAmbientColorTemperature;
    
    Format                 mFormat;
    
    bool                   mIsRunning = false;
    
    vec2                   mCameraSize = vec2( 1.0f );
    
    gl::GlslProgRef        mYCbCrToRGBProg;
    
};
} // namespace ARKit

#endif /* CinderARKit_h */
