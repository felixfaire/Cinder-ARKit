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

/**  An anchor point that will be tracked by ARKit*/
struct Anchor
{
    mat4           mTransform;
};

/**  An anchor point that includes plane position and extents*/
struct PlaneAnchor : public Anchor
{
    vec2 mCenter;
    vec2 mExtents;
};

class Session
{
public:
    
    Session();
    Session( const Format options );
    ~Session();
    
    /**  Adds an anchor point to the ARSession relative from the current camera position
         and orienation.
    */
    void addAnchorRelativeToCamera( vec3 offset );
    
    /**  Sets the size and aspect ratio of the view to calculate the projection matrix
    */
    void setViewSize( ivec2 newSize )   { mViewSize = newSize; }

    /**  Returns the texture for the current frame capture
    */
    gl::Texture2dRef getFrameTexture()  { return gl::Texture2d::create( mCurrentFrame ); }

    // Currently members are publically exposed to Obj C Implementation
    
    std::map<std::string, Anchor>  mAnchorMap;
    
    cinder::Surface8u      mCurrentFrame;

    mat4                   mViewMatrix;
    mat4                   mProjectionMatrix;
    
    vec2                   mViewSize = vec2(1.0f);
    
    
};
} // namespace ARKit

#endif /* CinderARKit_h */
