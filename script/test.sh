#!/bin/bash
# must be run from the top-level of the max-mxj dir

# 1. Copy the MXJ package into the install location
# Done in the travis script where the $PACKAGE_NAME var is in-scope

# 2. Download and copy the Test framework package into the install location

# NOTE: picking up Nick's interim release, to get the library patchers. And the packing script
# (zip-it.sh in max-test) leaves the path prefix as "max-test".

#curl https://github.com/Cycling74/max-test/releases/download/v1.0-beta3/max-test-v1.0-beta3.zip -L --output max-test.zip
curl https://github.com/cassiel/max-test/releases/download/v1.1-beta1/max-test-v1.1-beta1.zip -L --output max-test.zip
unzip max-test.zip -d Packages
cp -r Packages/max-test ~/Documents/Max\ 7/Packages/Max-Test

# 3. Download and install Max
curl http://akiaj5esl75o5wbdcv2a-maxmspjitter.s3.amazonaws.com/Max724_160725.dmg --output max.dmg
hdiutil attach max.dmg
mkdir -p ~/Applications
cp -r /Volumes/Max7_160721_1cf59cb/Max.app ~/Applications

# 4. Remove the pre-installed MXJ so the we know for certain that our build will be used
rm -rf ~/Applications/Max.app/Contents/Resources/C74/packages/max-mxj
#cp -rv $PACKAGE_NAME ~/Applications/Max.app/Contents/Resources/C74/packages

# 5. Configure
cd ~/Documents/Max\ 7/Packages/Max-Test
mv misc/testpackage-config-example.json misc/testpackage-config.json
rm -rf patchers
cd ruby
rvm get head
gem install sqlite3

# Diagnostic
ls -la ~/Documents/Max\ 7/Packages/
ls -la ~/Documents/Max\ 7/Packages/*
ls -la ~/Documents/Max\ 7/Packages/*/patchers
java -version

# 6. Run the tests
./test.rb ~/Applications
