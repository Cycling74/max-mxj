#!/bin/bash

# Pack .class files for project $1 into lib, then delete them.

echo "Converting $1 to JAR"

cd $1/code

classes=$(find . -name '*.class')

mkdir -p lib
jar cvf lib/$1.jar $classes
rm $classes
