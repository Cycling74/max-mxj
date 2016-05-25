# max-mxj
[![Build Status](https://travis-ci.com/Cycling74/max-mxj.svg?token=GAmnsUEo9aYasSF5pz8q&branch=master)](https://travis-ci.com/Cycling74/max-mxj)
[![Build status](https://ci.appveyor.com/api/projects/status/gp3t8xshfsjbmdcy?svg=true)](https://ci.appveyor.com/project/c74/max-mxj)

The mxj/mxj~ objects for authoring objects in Max using Java.


## Continuous Integration

Builds can be downloaded from [S3](https://s3-us-west-2.amazonaws.com/cycling74-ci/index.html?prefix=max-mxj/)

Use the badges above to go directly to the CI services.


## To build (Mac)

* `mkdir build`
* `cd build`
* `cmake -G Xcode ..`
* `cmake --build .` (or open the Xcode project in this build folder and build there)


## To build (Windows)

Download the Java 2 SDK from Oracle, and place it in the top-level "source" folder. It should be named "j2sdk" and contain at least 3 folders named "include", "lib", and "jre".

Open the mxj or mxj~ project in Visual Studio 2013
