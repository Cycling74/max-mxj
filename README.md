# max-mxj
[![Build Status](https://travis-ci.com/Cycling74/max-mxj.svg?token=GAmnsUEo9aYasSF5pz8q&branch=master)](https://travis-ci.com/Cycling74/max-mxj)
[![Build status](https://ci.appveyor.com/api/projects/status/gp3t8xshfsjbmdcy?svg=true)](https://ci.appveyor.com/project/c74/max-mxj)

The mxj/mxj~ objects for authoring objects in Max using Java.

## jb/cmake

This is an experimental branch with true Cmake build support (previously, Cmake would run platform-specific, static projects).

Recent versions of macOS no longer ship with a JavaVM.framework, and recent versions of Xcode no longer include that framework in the SDK. For that reason, this branch assumes a complete JDK install with `java_home` located at `/usr/libexec/java_home`. The build instructions remain otherwise the same.

On Windows, you'll need to install the JDK as well as Cmake, but the build instructions are identical to those on macOS.

## Continuous Integration

Builds can be downloaded from [S3](https://s3-us-west-2.amazonaws.com/cycling74-ci/index.html?prefix=max-mxj/)

Use the badges above to go directly to the CI services.


## To build (macOS)

* `mkdir build`
* `cd build`
* `cmake -G Xcode ..`
* `cmake --build .` (or open the Xcode project in this build folder and build there)
* `cmake --build . --config Release`


## To build (Windows)
Download the Windows x86 Java SE Development Kit 8 (from http://www.oracle.com and install it in the default location (C:\Program Files (x86)\Java). Rename 'C:\Program Files (x86)\Java\jdk1.8.0_xxx' to 'C:\Program Files (x86)\Java\jdk1.8.0'.

Download and install Cmake.

Build as on macOS.

## MXJ rules for searching for Java:
On OSX, it searches in that order: embeded JRE, on system JDK, on system JRE
The search for an embeded JRE is done in the application, in the folder MyApp.app/Contents/Resources/C74/packages/max-mxj/jre
The JRE must have the structure :
jre/Contents/Home
jre/Contents/MacOS
jre/Contents/Plugins
jre/Contents/Resources
jre/Contents/Frameworks

The JRE can be found here after installing a java internet plugin:
"/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/"
Copy that folder and rename it "jre"

On Windows, it searches in that order: embeded JRE, on system JDK, on system JRE
The search for an embeded JRE is done in the application, it must be a folder named "jre" placed in the same folder as the application .exe 
