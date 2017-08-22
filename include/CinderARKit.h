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

/**  An anchor point that will be tracked by ARKit*/
class Anchor
{
public:
    Anchor() {}
    Anchor( mat4 transform )
        : mTransform( transform ) {}
    
    mat4           mTransform;
};

/**  An anchor point that includes plane position and extents*/
class PlaneAnchor
{
public:
    PlaneAnchor(){}
    PlaneAnchor( mat4 transform, vec3 center, vec3 extent )
        : mTransform( transform ),
          mCenter( center ),
          mExtent( extent ) {}
    
    mat4 mTransform;
    vec3 mCenter;
    vec3 mExtent;
};

class Session
{
public:
    
    enum TrackingConfiguration
    {
        OrientationTracking,
        WorldTracking,
        WorldTrackingWithHorizontalPlaneDetection
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
    
    /**  Adds an anchor point to the ARSession relative from the current camera position
         and orienation.
    */
    void addAnchorRelativeToCamera( vec3 offset );

    /**  Returns the luma texture for the current frame capture
    */
    gl::Texture2dRef getFrameLumaTexture()    { return gl::Texture2d::create( mFrameYChannel ); }
    
    /**  Draws the converted rgb capture to the desired area
    */
    void drawRGBCaptureTexture( Area area );
    
    /**  Returns the estimated light intensity for the scene
         0.0 (very dark) 1.0 (very bright)
    */
    float getAmbientLightIntensity()        { return mAmbientLightIntensity; }
    
    /**  Returns the estimated light color temperature for the scene in Kelvin
         (6500 is pure white)
    */
    float getAmbientColorTemperature()      { return mAmbientColorTemperature; }
    
    /**  Get the list of anchor transforms (points with orientations)
    */
    std::vector<Anchor> getAnchors();
    
    /**  Get the list of plane anchor abjoects
    */
    std::vector<PlaneAnchor> getPlaneAnchors();
    

    // Currently members are publically exposed to Obj C Implementation
    
    std::map<std::string, Anchor>       mAnchors;
    std::map<std::string, PlaneAnchor>  mPlaneAnchors;
    
    Channel8u              mFrameYChannel;
    Channel8u              mFrameCbChannel;
    Channel8u              mFrameCrChannel;

    mat4                   mViewMatrix;
    mat4                   mProjectionMatrix;
    
    float                  mAmbientLightIntensity;
    float                  mAmbientColorTemperature;
    
    Format                 mFormat;
    
    bool                   mIsRunning = false;
    
    gl::GlslProgRef        mYCbCrToRGBProg;
    
};
} // namespace ARKit

#endif /* CinderARKit_h */
