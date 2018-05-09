# Cinder-ARKit

A simple bridge between Apples ARKit and Cinder.
(This block is very much beta software and does not yet expose all the new ARKit functionality, all suggestions / contributions welcome).

## Setup Notes:

- You must have Xcode 9.0 and iOS 11.0 or later to use ARKit (11.3 or later to use Image Tracking)
- In your xcode project General settings change the team to your personal development team. Also set your Deployment Target to ios 11.3.
- In your project info.plist file add a new "Privacy - Camera Usage Description" key (so your app can use the camera).
- Run the BasicSample app to get an idea of how the Session object works.
- Check out CinderARkit.h for more information.

## General Functionality:
The functionality of this cinder block tries to stay fairly close to the original API so you can find more top level info of what ARKit provides here https://developer.apple.com/arkit/

### - ARKit Session
This is the primary interface to ARKit. It wont do anything until you call runConfiguration() with a SessionConfiguration, this specifies whether you want WorldTracking, OrientationTracking, PlaneDetection etc.

### - AR Camera View and Projection matrices:
These allow you to track your phones real world 3D position and orientation in virtual 3D space. All matrix translation units are in real world meters.

### - Anchors:
You can add tracked 3D positions (including their orientation) to your scene relative to the current camera position (or straight in AR world coordinates).
(These points will be updated / adjusted behind the scenes by ARKit to provide the most coherent / accurate spatial model).

### - PlaneAnchors:
These are plane anchor points that the ARKit Session will provide you when a new horizontal plane has been detected. These anchor points feature a center position and a size as well as the AnchorPoint orientation transform.
(If you you must run the ARKitSession with the WorldTrackingWithHorizontalPlaneDetection configuration in order to detect these, though this is enabled by default)

### - Light Estimate:
The Session features a light intensity estimate (0.0 for very dark and 1.0 for very bright scenes) and a [color temperature estimate in Kelvin](https://en.wikipedia.org/wiki/Color_temperature) (6500 is pure white). You can use these values to adjust lighting and shading properties in your scene.

### - Image Capture
The Session provides a convenience function to draw the camera capture directly to the screen so you can overlay 3D information on the camera image.
(The Session has to convert the captured image from YCrCb space to RGB and uses in built shaders to do this, if you want an RGB texture of the camera capture I would suggest using this function to render to an Fbo and then using the texture from that)

### - Image Tracking:
ARKit can recognise images in the real work and create ImageAnchors from them in virtual space. See the ImageTracking example for more.
