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

#define MXJ_JAVA_PATH_MAX_LEN 4096

int isVMLibrary(_TCHAR *vm);


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
char dirSeparator = '/';

#define JAVA_FRAMEWORK "/System/Library/Frameworks/JavaVM.framework"

#define MAX_LOCATION_LENGTH 100 /* none of the jvmLocations strings should be longer than this */
#define MAX_JVMLIB_LENGTH   100 /* none of the jvmLibs strings should be longer than this */
static const char *jvmLocations[] = {
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
static const char *jvmLibs[]      = {"libclient64.dylib", "libjvm.dylib", "libjvm.jnilib", "libjvm.so", NULL};

/* Define the window system arguments for the various Java VMs. */
static const char *argVM_JAVA[] = {"-XstartOnFirstThread", NULL};

static const char *mxjSuffixes[] = {
        "externals/mxj.mxo/Contents/MacOS/mxj\0",
        "externals/mxj~.mxo/Contents/MacOS/mxj~\0",
        "extensions/mxj_safe.mxo/Contents/MacOS/mxj_safe\0",
        NULL
};

static bool fileExists(const char *filename, bool isExecutable)
{
    int res = access(filename, F_OK | R_OK | (isExecutable ? X_OK : 0));
    return res == 0;
}

/** a function to determine if string ends with a specific value */
inline bool hasEnding(const char *fullString, const char *ending)
{
    size_t is = (fullString == NULL) ? 0 : strlen(fullString);
    size_t ie = strlen(ending);
    if (is < ie) { return false; }

    while (ie--)
    {
        is--;
        if (ending[ie] != fullString[is]) { return false; }
    }
    return true;
}

static char *privateEmbeddedHomeDirectory        = NULL;
static bool privateEmbeddedHomeDirectorySearched = false; // Search already done? avoid doing it several times

// Function to retreive eventually the existence of embedded JRE home directory
// (folder jre in max-mxj package)
char *getEmbeddedHomeDirectory()
{
    //for osx we only look at embedded binaries for 64 bit
#if !defined(__x86_64__)
    return NULL;
#else
    if (!privateEmbeddedHomeDirectorySearched) // Search embedded jre if not searched yet
    {
        privateEmbeddedHomeDirectorySearched = true;
        privateEmbeddedHomeDirectory         = NULL;

        //here we're going to find where our library file is on OSX using dladdr
        //this returns the executable based on a symbol in our memory
        Dl_info myPluginInfo;
        if (dladdr("mxj", &myPluginInfo) != 0)
        {
            if (myPluginInfo.dli_fname != NULL)
            {
                const char *jreHome = "jre/Contents/Home";

                // Check all possible suffixes (call can be made from mxj, mxj~ and mxj_safe objects)
                for (int q = 0; (mxjSuffixes[q] != NULL) && (privateEmbeddedHomeDirectory == NULL); ++q)
                {
                    const char *mxjSuffix = mxjSuffixes[q];
                    if (hasEnding(myPluginInfo.dli_fname, mxjSuffix))
                    {
                        const char *fullName = myPluginInfo.dli_fname; // Full binary path name
                        if (fullName != NULL)
                        {
                            // Replace mxj suffix in path by jre home relative path

                            const int maxxxLen = (int)(strlen(fullName) + strlen(jreHome) + 1); // In case mxjSuffix is smaller than jreHome, take precautions
                            char      embeddedHome[maxxxLen];
                            memset(embeddedHome, 0, maxxxLen); // Clear it
                            // Set to fullName
                            strncpy(embeddedHome, fullName, maxxxLen - 1); // Keep last 0
                            // Remove mxj suffix
                            embeddedHome[strlen(fullName) - strlen(mxjSuffix)] = 0;
                            // Append jre home
                            strncat(embeddedHome, jreHome, maxxxLen - 1); // Keep last 0

                            // Check for folder
                            if (fileExists(embeddedHome, true))
                            {
                                privateEmbeddedHomeDirectory = strdup(embeddedHome);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return privateEmbeddedHomeDirectory;
#endif
}


char * findLib(char * command) {
    int         i, q;
    int         pathLength;
    struct stat stats;
    char        *path; /* path to resulting jvm shared library */
    char        *location; /* points to begining of jvmLocations section of path */

    if (command != NULL) {
        /*check first to see if command already points to the library */
        if (isVMLibrary(command)) {
            if (stat(command, &stats) == 0 && (stats.st_mode & S_IFREG) != 0) { /* found it */
                return strdup(command);
            }
            return NULL;
        }

        location   = strrchr(command, dirSeparator) + 1;
        pathLength = (int)(location - command);
        path       = (char *) malloc((pathLength + MAX_LOCATION_LENGTH + 1 + MAX_JVMLIB_LENGTH + 1) * sizeof(char));
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
        
        free(path);
    }
    return NULL;
}


int isVMLibrary(_TCHAR *vm)
{
    _TCHAR *ch = NULL;
    if (vm == NULL) { return 0; }
    ch = _tcsrchr(vm, '.');
    if (ch == NULL)
    {
        return 0;
    }
#ifdef _WIN32
    return (_tcsicmp(ch, _T_ECLIPSE(".dll")) == 0);
#else
    return (_tcsicmp(ch, _T_ECLIPSE(".so")) == 0) || (_tcsicmp(ch, _T_ECLIPSE(".jnilib")) == 0) || (_tcsicmp(ch, _T_ECLIPSE(".dylib")) == 0);
#endif
}

char * getJavaVersion(char* command) {
    FILE *fp;
    char buffer[MXJ_JAVA_PATH_MAX_LEN];
    char *version = NULL, *firstChar;
    int  numChars = 0;
    sprintf(buffer, "%s -version 2>&1", command);
    fp = popen(buffer, "r");
    if (fp == NULL) {
        return NULL;
    }
    while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
        if (!version) {
            firstChar = (char *) (strchr(buffer, '"') + 1);
            if (firstChar != NULL)
            {
                numChars = (int) (strrchr(buffer, '"') - firstChar);
            }

            /* Allocate a buffer and copy the version string into it. */
            if (numChars > 0)
            {
                version = (char *) malloc(numChars + 1);
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

/** Search for a JRE internet plugin on the machine */
char *getJREHome()
{
#define JRE_PLUGIN_HOME "/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home"
    char path[MXJ_JAVA_PATH_MAX_LEN];
    snprintf(path, sizeof(path), JRE_PLUGIN_HOME "/bin/java");
    if (!fileExists(path, true))
    {
        // No JRE certainly
        return NULL;
    }

    // Return JRE home path
    return strdup(JRE_PLUGIN_HOME);
}

/** Search for a JDK on the machine */
char *getJDKHome()
{
    FILE *fp;
    char path[MXJ_JAVA_PATH_MAX_LEN];
    char *result, *start;
    snprintf(path, sizeof(path), "/usr/libexec/java_home -a %s", JAVA_HOME_ARCH);
    fp = popen(path, "r");
    if (fp == NULL)
    {
        // No JDK certainly
        return NULL;
    }

    // Build and return JDK home path
    while (fgets(path, sizeof(path) - 1, fp) != NULL) {}
    if (strstr(path, " -a "))
    {
        return NULL;
    }
    result = path;
    start  = strchr(result, '\n');
    if (start)
    {
        start[0] = 0;
    }

    return strdup(result);
}

/** Search for a JDK or JRE java home */
static char *privateJavaHomeDirectory        = NULL;
static bool privateJavaHomeDirectorySearched = false; // Search already done? avoid doing it several times

char *getHome()
{
    if (!privateJavaHomeDirectorySearched)
    {
        privateJavaHomeDirectorySearched = true;

        // Search embedded JRE (only works with x64)
        char *embeddedHome = getEmbeddedHomeDirectory();
        if (embeddedHome != NULL) { privateJavaHomeDirectory = embeddedHome; }
        else
        {
            // Nothing embedded, search JDK
            char *jdkHome = getJDKHome();
            if (jdkHome != NULL) { privateJavaHomeDirectory = jdkHome; }
            else
            {
                // No JDK, search JRE
                char *jreHome = getJREHome();
                privateJavaHomeDirectory = jreHome;
            }
        }
    }
    return privateJavaHomeDirectory;
}

char * getJavaHome()
{
    char path[MXJ_JAVA_PATH_MAX_LEN];
    char *home = getHome();
    if (home == NULL) { return NULL; }

    snprintf(path, sizeof(path), "%s/bin/java", home);
    return strdup(path);
}

char *getJavaJli()
{
    char path[MXJ_JAVA_PATH_MAX_LEN];
    char *home = getHome();
    if (home == NULL)
    {
        return NULL;
    }

    // Search JDK
    snprintf(path, sizeof(path), "%s/../MacOS/libjli.dylib", home);
    // Check if JDK jli is found
    if (!fileExists(path, false))
    {
        // Not found, search JRE
        // This is needed when embeddedHomeDirectory is not NULL, which means we found an embedded JRE (so no JDK at home path)
        snprintf(path, sizeof(path), "%s/lib/jli/libjli.dylib", home); // This is that path from at least JRE 8, compatible with osx 10.7.3+

        if (!fileExists(path, false))
        {
            return NULL; // Nothing found
        }
    }
    return strdup(path);
}

const char * findVMLibrary( char* command ) {
    char       *start, *end;
    char       *version, *cmd;
    int        length;
    const char *result;

    /*check first to see if command already points to the library */
    if (strcmp(command, JAVA_FRAMEWORK) == 0) {
        return JAVA_FRAMEWORK;
    }

    /* select a version to use based on the command */
    start = strstr(command, "/Versions/");
    if (start != NULL){
        start += 10;
        end = strchr(start, dirSeparator);
        if (end != NULL && end > start) {
            length  = (int)(end - start);
            version = (char *) malloc(length + 1);
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
        char *lib = findLib(cmd);
        if (lib != NULL) {
            result = lib;
        }
    }
    if (cmd != command) { free(cmd); }
    return result;
}






