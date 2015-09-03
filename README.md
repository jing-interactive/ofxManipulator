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


Examples
--------

### [example-ofxManipulator-SnowdenBust](https://github.com/ofnode/example-ofxManipulator-SnowdenBust)

![](https://github.com/ofnode/example-ofxManipulator-SnowdenBust/tree/master/gifs/translation-1.gif)
![](https://github.com/ofnode/example-ofxManipulator-SnowdenBust/tree/master/gifs/translation-2.gif)
![](https://github.com/ofnode/example-ofxManipulator-SnowdenBust/tree/master/gifs/rotation-1.gif)
![](https://github.com/ofnode/example-ofxManipulator-SnowdenBust/tree/master/gifs/rotation-2.gif)
![](https://github.com/ofnode/example-ofxManipulator-SnowdenBust/tree/master/gifs/scale-1.gif)
![](https://github.com/ofnode/example-ofxManipulator-SnowdenBust/tree/master/gifs/scale-2.gif)


Compiling
---------

For [openFrameworks](https://github.com/openframeworks/openFrameworks):

[See wiki](https://github.com/ofnode/of/wiki/Compiling-ofApp-with-vanilla-openFrameworks)

For [CMake-based openFrameworks](https://github.com/ofnode/of):

Add this repo as a git submodule to your [ofApp](https://github.com/ofnode/ofApp) folder and use `ofxaddon` command in `CMakeLists.txt`.

