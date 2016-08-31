#!/bin/bash

# Compile all source files under Java 6, OS X.
# Run in the directory containing this script.

BOOT_CLASSES=/System/Library/Frameworks/JavaVM.framework/Versions/1.6/Classes/classes.jar
MAX_JAR=/Applications/Max.app/Contents/Resources/C74/packages/max-mxj/java-classes/lib/max.jar

for f in $(find . -name '*.maxproj'); do
    code_root=${f%/*.maxproj}/code

    if [ -d $code_root ]; then
        echo ROOT: $code_root
        pushd $code_root
        sources=`find . -name '*.java'`
        #echo SOURCES: $sources
        javac -bootclasspath $BOOT_CLASSES -cp $MAX_JAR -source 6 -target 6 $sources
        popd
    fi
done

# However: specifically (re)compile this trap file at higher Java version, to see if we identify it:

NEWER=NewerFormat.java
echo "Compiling later version: " $NEWER
(cd check-class-file-version/code && javac -cp $MAX_JAR $NEWER)

# Hardwire any tests for generated JARs:

for f in qualified-class-name_JAR; do
    ./jarify.sh $f
done
