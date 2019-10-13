#!/bin/bash

# Compile all source files under Java 6, OS X.
# Run in the directory containing this script.

# NOTE: ../java-classes is considered expendible: we remove and recreate it on a compilation
# run.


BOOT_CLASSES=/System/Library/Frameworks/JavaVM.framework/Versions/1.6/Classes/classes.jar
#MAX_NAME=Max_ed23618
MAX_NAME=Max
MAX_JAR=/Applications/${MAX_NAME}.app/Contents/Resources/C74/packages/max-mxj/java-classes/lib/max.jar

CLASSES=`find . -name '*.class'`
SOURCES=`find . -name '*.java'`

rm -r $CLASSES
javac \
    -bootclasspath $BOOT_CLASSES \
    -cp $MAX_JAR \
    -source 6 \
    -target 6 \
    $SOURCES

# However: specifically (re)compile this trap file at higher Java
# version, to see if we identify it: NOTE: we plant the files
# directly into the project, so that the patcher can find them,
# even though MXJ has trouble loading Java in projects. (We
# don't really mind which ones we're running.)

PROJECT=../patchers/check-class-file-version
CODE=$PROJECT/code
rm -rf $CODE
mkdir $CODE

NEWER=NewerFormat.java
echo "Compiling later version: " $NEWER
javac -cp $MAX_JAR -d $CODE $NEWER
# Copy in the one we actually compiled for running - we want to
# check it's not above Java 6.
cp CheckClassFileVersion.class $CODE/
