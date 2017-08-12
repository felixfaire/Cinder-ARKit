//
//  CinderARKitUtils.h
//  ARKitTest
//
//  Created by Felix Faire on 11/08/2017.
//

#ifndef CinderARKitUtils_h
#define CinderARKitUtils_h

/**
     Utils to convert between Metal objects and Cinder objects
*/

// Conversions to simd types
// Thanks to http://wdlindmeier.com/ !

template <typename T, typename U >
const U static inline convert( const T & t )
{
    U tmp;
    memcpy(&tmp, &t, sizeof(U));
    U ret = tmp;
    return ret;
}

const matrix_float4x4 static inline toMtl( const glm::mat4 & mat )
{
    return convert<glm::mat4, matrix_float4x4>(mat);
}

const glm::mat4 static inline fromMtl( const matrix_float4x4 & mat )
{
    return convert<matrix_float4x4, glm::mat4>(mat);
}

static mat4 modelMatFromTransform( matrix_float4x4 transform )
{
    matrix_float4x4 coordinateSpaceTransform = matrix_identity_float4x4;
    // Flip Z axis to convert geometry from right handed to left handed
    coordinateSpaceTransform.columns[2].z = -1.0;
    matrix_float4x4 modelMat = matrix_multiply(transform, coordinateSpaceTransform);
    return fromMtl(modelMat);
}

static const std::string getUidStringFromAnchor( ARAnchor* anchor )
{
    NSString* uid = anchor.identifier.UUIDString;
    return std::string([uid UTF8String], [uid lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
}

#endif /* CinderARKitUtils_h */
