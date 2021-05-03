//
//  OSXSys.mm
//  MAX MXJ
//
//  Created by PJ Slack on March 28 2017
//  Copyright 2017 MAPLEPOST
//  See attached MIT License
//

#include "OSXSys.h"
#import <Foundation/NSBundle.h>
#import <Appkit/NSRunningApplication.h>
#include <errno.h>
#include <sys/sysctl.h>
#include <string>

bool OSXSys::is64BitRunning(){

	switch ([[NSRunningApplication currentApplication] executableArchitecture]) {
		case NSBundleExecutableArchitectureI386:
			return false;
			break;
			
		case NSBundleExecutableArchitectureX86_64:
			return true;
			break;
			
		case NSBundleExecutableArchitecturePPC:
			return false;
			break;
			
		case NSBundleExecutableArchitecturePPC64:
			return true;
			break;

		case NSBundleExecutableArchitectureARM64:
			return true;
			break;
			
		default:
			return false;
			break;
	}
    
	
}


OSXSys::OSXSys()
{

    
    size_t size = sizeof(osrelease);
    bzero(osrelease,size);
    majorOSVersion = 0 ;
    int ret = sysctlbyname("kern.osrelease", osrelease, &size, NULL, 0);
    
    //non zero is an error
    if(ret != 0)
    {
    
    }
    else
    {
        std::string s(osrelease);
        std::string delimiter = ".";
        std::string token = s.substr(0, s.find(delimiter));
        majorOSVersion=atoi(token.c_str());
        
    }
    
    
}


int OSXSys::getMajorOSVersion()
{
    return majorOSVersion;
}
