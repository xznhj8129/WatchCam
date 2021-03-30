# WatchCam<br>
## Version 1.0
Contour-based OpenCV security system. Ported from Python 2 to C++ for a 10x CPU usage improvement. This is my first serious C++ software ever. Very basic but reliable and had been in use for over a year for real-life security monitoring. First version used simple image diff threshold and was too oversensitive to changes in light and artifacts, new contour-detection size-trigger based algorithm was far superior. Image downsizing + greyscale + light gaussian blurring has removed practically all image artifact and false movement.

Has a simple arduino connection that makes a led illuminate or flash depending on the tracked object. You could use that as base for more advanced alerts like a siren, radio beeper, security drones or heat-seeking missiles or whatever.

### Config file
Support for 4 cams for now<br>
Important variables for motion algorithm:
- min_contour_size
- binarize_threshold
- blur_aperture
- md_scale (will be dynamically calculated soon)

### Requirements
- OpenCV4
- libserial
- libconfig
- PySerial (to be dropped)

### To be added:
- Adding rectangle select motion zone
- Alert sound playing
- ffmpeg HD recording
- Windows support
- HD picture dynamic scaling
- C++ Arduino controller
