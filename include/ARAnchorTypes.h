//
//  CinderARKit.h
//  ARKitTest
//
//  Created by Felix Faire on 11/08/2017.
//

#ifndef ARAnchorTypes_h
#define ARAnchorTypes_h

#include "cinder/gl/gl.h"

using namespace ci;

namespace ARKit {

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

/**  An anchor point from a tracked image*/
class ImageAnchor
{
public:
    ImageAnchor(){}
    ImageAnchor(  std::string uid, mat4 transform, vec2 physicalSize, std::string imageName )
        : mUid( uid ),
        mTransform( transform ),
        mPhysicalSize( physicalSize ),
        mImageName (imageName){}
    
    AnchorID        mUid;
    mat4            mTransform;
    vec2            mPhysicalSize;
    std::string     mImageName;
    
};

} // namespace ARKit

#endif /* ARAnchorTypes_h */
