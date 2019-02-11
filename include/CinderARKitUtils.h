//
//  CinderARKitUtils.h
//  ARKitTest
//
//  Created by Felix Faire on 11/08/2017.
//

#ifndef CinderARKitUtils_h
#define CinderARKitUtils_h

#if DEBUG
  #define DBG_ASSERT(expression) if (! (expression)) ::kill (0, SIGTRAP);
#else
  #define DBG_ASSERT(expression)
#endif

/**
     Utils to convert between iOS metal vector types and glm types
*/

// Conversions to simd types
// By William Lindmeier http://wdlindmeier.com/

namespace ARKit {

template <typename T, typename U >
const U static inline convert( const T& t )
{
    U tmp;
    memcpy(&tmp, &t, sizeof(U));
    U ret = tmp;
    return ret;
}

const glm::mat4 static inline toMat4( const matrix_float4x4& mat ) { return convert<matrix_float4x4, glm::mat4>(mat); }
const glm::vec4 static inline toVec4( const vector_float4& vec )   { return convert<vector_float4, glm::vec4>(vec); }
const glm::vec3 static inline toVec3( const vector_float3& vec )   { return convert<vector_float3, glm::vec3>(vec); }
const glm::vec2 static inline toVec2( const vector_float2& vec )   { return convert<vector_float2, glm::vec2>(vec); }

static mat4 modelMatFromTransform( matrix_float4x4 transform )
{
    matrix_float4x4 coordinateSpaceTransform = matrix_identity_float4x4;
    // Flip Z axis to convert geometry from right handed to left handed
    coordinateSpaceTransform.columns[2].z = -1.0;
    matrix_float4x4 modelMat = matrix_multiply(transform, coordinateSpaceTransform);
    return toMat4( modelMat );
}

typedef std::string AnchorID;
template <typename AnchorType>
static void updateOrAddAnchor( std::vector<AnchorType>& anchorList, AnchorType anchor )
{
    const auto anchorID = anchor.mUid;
    auto foundAnchor = std::find_if( anchorList.begin(),
                                     anchorList.end(),
                                     [anchorID](const AnchorType& a) -> bool { return (a.mUid == anchorID);} );
    
    if (foundAnchor != anchorList.end())
        *foundAnchor = anchor;
    else
        anchorList.push_back(anchor);
}

template <typename AnchorType>
static void removeAnchorWithID( std::vector<AnchorType>& anchors, AnchorID anchorID )
{
    std::remove_if( anchors.begin(), anchors.end(), [anchorID](const AnchorType& a) -> bool { return (a.mUid == anchorID); } );
}

static const std::string getUidStringFromUUID( NSUUID* uid)
{
    NSString* uidString = uid.UUIDString;
    return std::string([uidString UTF8String], [uidString lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
}

static const std::string getUidStringFromAnchor( ARAnchor* anchor )
{
    return getUidStringFromUUID( anchor.identifier );
}


static ARConfiguration* getNativeARConfiguration( ARKit::SessionConfiguration config )
{
    if (config.mTrackingType == ARKit::TrackingType::OrientationTracking)
    {
        if (config.mPlaneDetection != PlaneDetection::None)
        {
            cinder::app::console() << "Error: Cannot use plane detection with OrientationTracking, use WorldTracking instead" << std::endl;
            DBG_ASSERT(false);
        }
        
        if (config.mImageTrackingEnabled != false)
        {
            cinder::app::console() << "Error: Cannot use image tracking with OrientationTracking, use WorldTracking instead" << std::endl;
            DBG_ASSERT(false);
        }
    
        return [AROrientationTrackingConfiguration new];
    }
    
    DBG_ASSERT(config.mTrackingType == ARKit::TrackingType::WorldTracking);
    
    ARConfiguration* arConfig;
    ARWorldTrackingConfiguration* wtc = [ARWorldTrackingConfiguration new];

    switch (config.mPlaneDetection)
    {
        case (ARKit::PlaneDetection::None):
          wtc.planeDetection = ARPlaneDetectionNone;
          break;
          
        case (ARKit::PlaneDetection::Horizontal):
          wtc.planeDetection = ARPlaneDetectionHorizontal;
          break;
          
        case (ARKit::PlaneDetection::Vertical):
          wtc.planeDetection = ARPlaneDetectionVertical;
          break;
    }
    
    if (config.mImageTrackingEnabled)
        wtc.detectionImages = [ARReferenceImage referenceImagesInGroupNamed:@"AR Resources" bundle: nil];
    
    arConfig = wtc;
    
    return arConfig;
}

static cinder::Channel8u getChannelForCVPixelBuffer( const CVPixelBufferRef& pixelBuffer, int planeIndex, int increment = 1, int offset = 0 )
{
    CVPixelBufferLockBaseAddress( pixelBuffer, kCVPixelBufferLock_ReadOnly );
    uint8_t* data = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane( pixelBuffer, planeIndex );
    const size_t bytesPerRow = CVPixelBufferGetBytesPerRowOfPlane( pixelBuffer, planeIndex );
    const size_t width = CVPixelBufferGetWidthOfPlane( pixelBuffer, planeIndex );
    const size_t height = CVPixelBufferGetHeightOfPlane( pixelBuffer, planeIndex );
    const auto channel = cinder::Channel8u( (int32_t)width, (int32_t)height, (int32_t)bytesPerRow, increment, data + offset );
    CVPixelBufferUnlockBaseAddress( pixelBuffer, kCVPixelBufferLock_ReadOnly );
    
    return channel;
}

} // namespace ARKit

#endif /* CinderARKitUtils_h */
