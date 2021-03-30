# WatchCam
## Version 1.0
Contour-based OpenCV security system. Ported from Python 2 to C++ for a 10x CPU usage improvement. This is my first serious C++ software ever. Very basic but reliable and had been in use for over a year for real-life security monitoring. First version used simple image diff  and was too oversensitive to changes in light, new contour-detection size-trigger based algorithm was far superior.

Has a simple arduino connection that makes a led illuminate or flash depending on the tracked object. You could use that as base for more advanced alerts like a siren, radio beeper or missiles or whatever.

### Config file
Support for 4 cams for now


### To be added:
- Adding rectangle select motion zone
- Motion triggered ffmpeg recording
- Windows support
- Full HD picture
