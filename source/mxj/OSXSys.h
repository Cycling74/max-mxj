//
//  OSXSys.mm
//  MAX MXJ
//
//  Created by PJ Slack on March 28 2017
//  Copyright 2017 MAPLEPOST
//  See attached MIT License
// TODO : if winodws implemetation is neccesarry expand functions to include windows particulars
// this version is to address a critical issues on OSX that has no support for 32 bit
//

class OSXSys {
public:

    OSXSys();

    /** returns true if running in 64 bit mode, only works for OSX currently, will always return fals on windows */
    bool is64BitRunning();
    /** this is poulated when the object is created with the version string */
    char osrelease[256];
    /** converts the version string to extract an int representing the major version number */
    int getMajorOSVersion();

private:
    int majorOSVersion;
    
    
};
