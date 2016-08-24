#!/bin/bash

# Compile all source files under Java 6, OS X.

BOOT_CLASSES=/System/Library/Frameworks/JavaVM.framework/Versions/1.6/Classes/classes.jar
MAX_JAR=/Applications/Max.app/Contents/Resources/C74/packages/max-mxj/java-classes/lib/max.jar

for f in $(find . -name '*.java'); do
    echo $f ...
    dir=$(dirname $f)
    file=$(basename $f)
    (cd $dir && javac -bootclasspath $BOOT_CLASSES -cp $MAX_JAR -source 6 -target 6 $file)
done
