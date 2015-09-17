ofxManipulator
==============

![](splash_image.png)

3D manipulator for moving objects in space. 
Partially integrates code from [LibGizmo](https://github.com/CedricGuillemet/LibGizmo) with some exceptions:

* Math is rewritten in openFrameworks.
* Works for both OpenGL 2.1 and 3.3+ renderers.
* Scaling, rotation and translation work properly together.
* You can move object in screen space by dragging from center of a translation manipulator.


Dependencies
------------

#### 1. OF 0.9.0 and C++11


Compiling
---------

For [openFrameworks](https://github.com/openframeworks/openFrameworks):

[See wiki](https://github.com/ofnode/of/wiki/Compiling-ofApp-with-vanilla-openFrameworks)

For [CMake-based openFrameworks](https://github.com/ofnode/of):

Add this repo as a git submodule to your [ofApp](https://github.com/ofnode/ofApp) folder and use `ofxaddon` command in `CMakeLists.txt`.


Examples
--------

### [example-ofxManipulator-SnowdenBust](https://github.com/ofnode/example-ofxManipulator-SnowdenBust)

<img src="https://raw.githubusercontent.com/ofnode/example-ofxManipulator-SnowdenBust/master/gifs/translation-1.gif" width="48%"><img src="https://raw.githubusercontent.com/ofnode/example-ofxManipulator-SnowdenBust/master/gifs/translation-2.gif" width="48%">
<img src="https://raw.githubusercontent.com/ofnode/example-ofxManipulator-SnowdenBust/master/gifs/rotation-1.gif" width="48%"><img src="https://raw.githubusercontent.com/ofnode/example-ofxManipulator-SnowdenBust/master/gifs/rotation-2.gif" width="48%">
<img src="https://raw.githubusercontent.com/ofnode/example-ofxManipulator-SnowdenBust/master/gifs/scale-1.gif" width="48%"><img src="https://raw.githubusercontent.com/ofnode/example-ofxManipulator-SnowdenBust/master/gifs/scale-2.gif" width="48%">
