//
//  JavaHomeOsx.c
//  mxj
//  Modified from Eclipse Launcher (attribution below)
//  Created by Peter Slack on 2015-10-17.
//  Copyright (c) 2015 Peter Slack ,Maplepost. All rights reserved.
//
/*
 * Copyright (c) 2006, 2009 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    IBM Corporation - initial API and implementation
 *	  Andrew Niefer
 */



#include "JavaHomeOsx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#include <dlfcn.h>

int isVMLibrary( _TCHAR* vm );

	
#ifdef i386
#define JAVA_ARCH "i386"
#define JAVA_HOME_ARCH "i386"
#elif defined(__amd64__) || defined(__x86_64__)
#define JAVA_ARCH "amd64"
#define JAVA_HOME_ARCH "x86_64"
#else
#define JAVA_ARCH DEFAULT_OS_ARCH
#define JAVA_HOME_ARCH DEFAULT_OS_ARCH
#endif

bool isSUN;
char   dirSeparator  = '/';

#define JAVA_FRAMEWORK "/System/Library/Frameworks/JavaVM.framework"

#define MAX_LOCATION_LENGTH 100 /* none of the jvmLocations strings should be longer than this */
#define MAX_JVMLIB_LENGTH   100 /* none of the jvmLibs strings should be longer than this */
static const char* jvmLocations[] = {
    "../lib/" JAVA_ARCH "/client",
    "../lib/" JAVA_ARCH "/server",
    "../lib/client",
    "../lib/server",
    "../jre/lib/" JAVA_ARCH "/client",
    "../jre/lib/" JAVA_ARCH "/server",
    "../jre/lib/client",
    "../jre/lib/server",
    "../bundle/Libraries",
    NULL
};
static const char* jvmLibs[] = { "libclient64.dylib","libjvm.dylib", "libjvm.jnilib", "libjvm.so", NULL };

/* Define the window system arguments for the various Java VMs. */
static char*  argVM_JAVA[] = { "-XstartOnFirstThread", NULL };

char * embeddedHomeDirectory=NULL;

char * findLib(char * command) {
    int i, q;
    int pathLength;
    struct stat stats;
    char * path; /* path to resulting jvm shared library */
    char * location; /* points to begining of jvmLocations section of path */
    
    if (command != NULL) {
        /*check first to see if command already points to the library */
        if (isVMLibrary(command)) {
            if (stat(command, &stats) == 0 && (stats.st_mode & S_IFREG) != 0) { /* found it */
                return strdup(command);
            }
            return NULL;
        }
        
        location = strrchr(command, dirSeparator) + 1;
        pathLength = location - command;
        path = (char*)malloc((pathLength + MAX_LOCATION_LENGTH + 1 + MAX_JVMLIB_LENGTH	+ 1) * sizeof(char));
        strncpy(path, command, pathLength);
        location = &path[pathLength];
        
        /*
         * We are trying base/jvmLocations[*]/vmLibrary
         * where base is the directory containing the given java command, normally jre/bin
         */
        for (q = 0; jvmLibs[q] != NULL; ++q) {
            const char *jvmLib = jvmLibs[q];
            i = -1;
            while (jvmLocations[++i] != NULL) {
                sprintf(location, "%s%c%s", jvmLocations[i], dirSeparator, jvmLib);
                /*fprintf(stderr,"checking path: %s\n",path);*/
                if (stat(path, &stats) == 0 && (stats.st_mode & S_IFREG) != 0)
                { /* found it */
                    return path;
                }
            }
        }
    }
    return NULL;
}


int isVMLibrary( _TCHAR* vm )
{
    _TCHAR *ch = NULL;
    if (vm == NULL) return 0;
    ch = _tcsrchr( vm, '.' );
    if(ch == NULL)
        return 0;
#ifdef _WIN32
    return (_tcsicmp(ch, _T_ECLIPSE(".dll")) == 0);
#else
    return (_tcsicmp(ch, _T_ECLIPSE(".so")) == 0) || (_tcsicmp(ch, _T_ECLIPSE(".jnilib")) == 0) || (_tcsicmp(ch, _T_ECLIPSE(".dylib")) == 0);
#endif
}

char * getJavaVersion(char* command) {
    FILE *fp;
    char buffer[4096];
    char *version = NULL, *firstChar;
    int numChars = 0;
    sprintf(buffer,"%s -version 2>&1", command);
    fp = popen(buffer, "r");
    if (fp == NULL) {
        return NULL;
    }
    while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
        if (!version) {
            firstChar = (char *) (strchr(buffer, '"') + 1);
            if (firstChar != NULL)
                numChars = (int)  (strrchr(buffer, '"') - firstChar);
            
            /* Allocate a buffer and copy the version string into it. */
            if (numChars > 0)
            {
                version = (char *) malloc( numChars + 1 );
                strncpy(version, firstChar, numChars);
                version[numChars] = '\0';
            }
        }
        if (strstr(buffer, "Java HotSpot(TM)") || strstr(buffer, "OpenJDK")) {
            isSUN = 1;
            break;
        }
        if (strstr(buffer, "IBM") != NULL) {
            isSUN = 0;
            break;
        }
    }
    pclose(fp);
    return version;
}

char * getHome()
{
    
    
    FILE *fp;
    char path[4096];
    char *result, *start;

    //for osx we only look at embedded binaries for 64 bit
#if defined(__x86_64__)
    if(embeddedHomeDirectory==NULL){
#endif
        sprintf(path, "/usr/libexec/java_home -a %s", JAVA_HOME_ARCH);
        fp = popen(path, "r");
        if (fp == NULL) {
            return NULL;
        }
        while (fgets(path, sizeof(path)-1, fp) != NULL) {
        }
        result = path;
        start = strchr(result, '\n');
        if (start) {
            start[0] = 0;
        }
        return strdup(result);

#if defined(__x86_64__)
    }

    else
    {
        return embeddedHomeDirectory;
    }
#endif

}

char * getJavaHome() {
    char path[4096];
    char * home = getHome();
    if(home == NULL)
        return NULL;
    
    sprintf(path, "%s/bin/java", home);
    return strdup(path);
}

char * getJavaJli()
{
    char path[4096];
    char * home = getHome();
    if(home == NULL)
        return NULL;
    
    sprintf(path, "%s/../MacOS/libjli.dylib", home);
    return strdup(path);

}


char * findVMLibrary( char* command ) {
    char *start, *end;
    char *version, *result, *cmd;
    int length;
    
    /*check first to see if command already points to the library */
    if (strcmp(command, JAVA_FRAMEWORK) == 0) {
        return JAVA_FRAMEWORK;
    }
    
    /* select a version to use based on the command */
    start = strstr(command, "/Versions/");
    if (start != NULL){
        start += 10;
        end = strchr( start, dirSeparator);
        if (end != NULL && end > start) {
            length = end - start;
            version = (char*)malloc(length + 1);
            strncpy(version, start, length);
            version[length] = 0;
            
            /*only set a version if it starts with a number */
            if(strtol(version, NULL, 10) != 0 || version[0] == '0') {
                setenv("JAVA_JVM_VERSION", version, 1);
            }
            
            free(version);
        }
    }
    cmd = command;
    if (strstr(cmd, "/JavaVM.framework/") != NULL && (strstr(cmd, "/Current/") != NULL || strstr(cmd, "/A/") != NULL)) {
        cmd = getJavaHome();
    }
    // This is necessary to initialize isSUN
    getJavaVersion(cmd);
    result = JAVA_FRAMEWORK;
    if (strstr(cmd, "/JavaVM.framework/") == NULL) {
        char * lib = findLib(cmd);
        if (lib != NULL) {
            result = lib;
        }
    }
    if (cmd != command) free(cmd);
    return result;
}






