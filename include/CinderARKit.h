//
//  CinderARKit.h
//  ARKitTest
//
//  Created by Felix Faire on 11/08/2017.
//

#ifndef CinderARKit_h
#define CinderARKit_h

#include "cinder/gl/gl.h"
#include "ARSessionImpl.h"

using namespace ci;

/**
     CinderARKit is a WIP bridge to expose the native
     AR functionality of iOS 11 in Cinder.
*/
namespace ARKit {

class Session;
typedef std::shared_ptr<Session> SessionRef;

class Session
{
public:
    
    Session();
    
    /**  Starts the ARSession with a configuration
    */
    void runConfiguration( TrackingConfiguration config );
    
    /**  Pauses the session
    */
    void pause();
    
    //===== AR Anchors =========================================================//
    /**  Adds an anchor point to the ARSession relative to the current world orientation.
         Returns the AnchorID of the anchor to reference later on.
    */
    const AnchorID addAnchorRelativeToWorld( vec3 position );
    
    /**  Adds an anchor point to the ARSession relative from the current camera position
         and orienation. Returns the AnchorID of the anchor to reference later on.
    */
    const AnchorID addAnchorRelativeToCamera( vec3 offset );
    
    /**  Get the list of anchor transforms (points with orientations)
    */
    const std::vector<Anchor>& getAnchors() const;
    
    /**  Get the list of plane anchor objects
    */
    const std::vector<PlaneAnchor>& getPlaneAnchors() const;
    
    /**  Finds the anchor with a certain ID if it exists. Returns an empty
         std::shared_ptr<Anchor> if it doesn't exist.
    */
    std::shared_ptr<Anchor> findAnchorWithID( AnchorID anchorID ) const;
    
    
    //===== Camera Matrices ====================================================//
    /**  Get the effective View matrix of the device camera
    */
    const mat4 getViewMatrix() const;
    
    /**  Get the effective Projection matrix of the device camera
    */
    const mat4 getProjectionMatrix() const;
    

    //===== Camera Capture =====================================================//
    /**  Set whether to process the colour image from the camera
    */
    void setRGBCaptureEnabled( bool captureEnabled );
    
    /**  Returns the luma texture for the current frame capture
    */
    gl::Texture2dRef getFrameLumaTexture() const;
    
    /**  Draws the converted rgb capture to the desired area
    */
    void drawRGBCaptureTexture( Area area ) const;
    
    /**  Returns the estimated light intensity for the scene
         0.0 (very dark) 1.0 (very bright)
    */
    float getAmbientLightIntensity() const;
    
    /**  Returns the estimated light color temperature for the scene in Kelvin
         (6500 is pure white)
    */
    float getAmbientColorTemperature() const;


private:

    static gl::GlslProgRef  createCameraRGBProg();

    // Internally handles bridge to objective-c
    SessionImpl             mSessionImpl;
    
    // Shaders to draw the camera image from YCbCr to RGB
    gl::GlslProgRef         mYCbCrToRGBProg;
    
    
    
};
} // namespace ARKit

#endif /* CinderARKit_h */
