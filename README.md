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
Download the Windows x64 Java SE Development Kit 8 (from http://www.oracle.com and install it in the default location (C:\Program Files\Java).

Download and install Cmake.
Define variable JAVA_HOME with `rundll32.exe sysdm.cpl,EditEnvironmentVariables`
* JAVA_HOME => "C:\Program Files\Java\jdk1.8.0_xxx"

* after that, launch a new shell, then from max-mxj folder:
* `mkdir build`
* `cd build`
* `set CUSTOM_FLAG="-DWIN64:Bool=True"`
* `cmake -G "Visual Studio 15 2017 Win64" ..`  (update version according to your Visual studio version, Win64 is mandatory)
* `cmake --build .` (or open the VS project in this build folder and build there)
* `cmake --build . --config Release`


## MXJ rules for searching for Java:

### MacOS
On OSX, it searches in that order: embedded JRE, on system JDK, on system JRE.

The search for an embedded JRE is done in the application, in the folder MyApp.app/Contents/Resources/C74/packages/max-mxj/jre
The JRE must have the structure for a single architecture:
jre/Contents/Home
jre/Contents/MacOS
jre/Contents/Plugins
jre/Contents/Resources
jre/Contents/Frameworks

For MacOS universal application, you must either use an universal JRE (no one seems to be available at the moment), or use one JRE per architecture:
jre/jre_x64/Contents/Home
jre/jre_x64/Contents/MacOS
jre/jre_x64/Contents/Plugins
jre/jre_x64/Contents/Resources
jre/jre_x64/Contents/Frameworks
jre/jre_aarch64/Contents/Home
jre/jre_aarch64/Contents/MacOS
jre/jre_aarch64/Contents/Plugins
jre/jre_aarch64/Contents/Resources
jre/jre_aarch64/Contents/Frameworks

If you want to use a zulu jre, you must extract its content to match the above structure.

The JRE can be found here after installing a java internet plugin:
"/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/"
Copy that folder and rename it "jre".

### Windows
On Windows, it searches in that order: embedded JRE, on system JDK, on system JRE
The search for an embedded JRE is done in the application, it must be a folder named "jre" placed in the same folder as the application .exe 

### JDK/JRE providers:
https://adoptium.net/temurin/releases/
https://adoptopenjdk.net/releases.html
https://www.oracle.com/java/technologies/downloads/

You can download a zipped version and rename the unzipped folder according to the requirements.
