#!/bin/bash
# must be run from the top-level of the max-mxj dir

# 1. Copy the MXJ package into the install location
mkdir -p ~/Documents/Max\ 7/Packages/
cp -r $PACKAGE_NAME ~/Documents/Max\ 7/Packages/

# 2. Download and copy the Test framework package into the install location
curl https://github.com/Cycling74/max-test/releases/download/v1.0-beta2/max-test-v1.0-beta2.zip -L --output max-test.zip
unzip max-test.zip -d Packages
cp -r Packages/max-test-v1.0-beta2 ~/Documents/Max\ 7/Packages/

# 3. Download and install Max
curl http://akiaj5esl75o5wbdcv2a-maxmspjitter.s3.amazonaws.com/Max724_160725.dmg --output max.dmg
hdiutil attach max.dmg
mkdir -p ~/Applications
cp -r /Volumes/Max7_160721_1cf59cb/Max.app ~/Applications

# 4. Remove the pre-installed MXJ so the we know for certain that our build will be used
rm -rf ~/Applications/Max.app/Contents/Resources/C74/packages/max-mxj
cp -rv $PACKAGE_NAME ~/Applications/Max.app/Contents/Resources/C74/packages

# 5. Configure
cd ~/Documents/Max\ 7/Packages/max-test-v1.0-beta2
mv misc/testpackage-config-example.json misc/testpackage-config.json
#rm -rf patchers
cd ruby
rvm get head
gem install sqlite3

# 6. Run the tests
./test.rb ~/Applications
