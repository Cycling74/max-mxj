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
Download the Windows x86 Java SE Development Kit 8 (from http://www.oracle.com and install it in the default location (C:\Program Files (x86)\Java). Rename 'C:\Program Files (x86)\Java\jdk1.8.0_xxx' to 'C:\Program Files (x86)\Java\jdk1.8.0'.

Open the mxj or mxj~ project in Visual Studio 2013
