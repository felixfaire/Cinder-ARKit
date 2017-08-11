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

class CinderARKit
{
public:
    CinderARKit();
    
    ~CinderARKit();
    
    /**  Adds an anchor point to the ARSession relative from the current camera position
         and orienation.
    */
    void addAnchorRelativeToCamera( vec3 offset );
    
    /**  Sets the size and aspect ratio of the view to calculate the projection matrix
    */
    void setViewSize( ivec2 newSize )   { mViewSize = newSize; }


    // Currently members are publically exposed to Obj C Implementation

    mat4 mViewMatrix;
    mat4 mProjectionMatrix;
    
    vec2 mViewSize = vec2(1.0f);
};

#endif /* CinderARKit_h */
