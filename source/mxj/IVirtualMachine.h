/*
 * IVirtualMachine.h
 *
 *  Created on: Feb 4, 2012
 *  Modified for Cycling '74 October 2015
 *      Author: Peter J Slack, P.Eng
 *  Copyright 2012-2015 WaveDNA Ltd. All rights reserved
 *
 */

#ifndef IVIRTUALMACHINE_H_
#define IVIRTUALMACHINE_H_

#include <stdio.h>
#include <iostream>
#include <list>
#include <string>
#include <stdlib.h>


//#define LLPLUGDEBUG

#ifdef WIN_VERSION
#include <jni.h>
#include <tchar.h>
#include <Windows.h>
#endif

#ifdef MAC_VERSION
#include <JavaVM/jni.h>
#include <unistd.h>
#include <strings.h>
#include "OSXSys.h"
extern "C"{
#include "JavaHomeOsx.h"
}
#endif


using namespace std;

/** a structure to allow us to firewall our c++ code and provide a c API interface */
struct ivirtualmachine{};

//-------------------------------------------------------------------------------------------------
// Class IVirtualMachine
//-------------------------------------------------------------------------------------------------

    class IVirtualMachine : public ivirtualmachine
{
public:
	//---------------------------------------------------------------------------------------------
	// Constructors and destructors
	//---------------------------------------------------------------------------------------------

	~IVirtualMachine();

	//---------------------------------------------------------------------------------------------
	// Public interface methods
	//---------------------------------------------------------------------------------------------

	static IVirtualMachine* getInstance();
	
	
	static bool exists();


	void *launchJVMThread(void * param);
	void addLibraryPath(string newPath);
	void addClassPath(string newPath);
	void addJavaOption(string newOption);
	void setStartupClass(string startClass);
	void startJava();
	void startJVM();
	void killJVM();
	string findEmbeddedLibrary(string appBase, bool sixtyfourbit);
	JavaVM * getJVM(){return jvm;};
    JNIEnv * attachJNIThread();
    
	IVirtualMachine(IVirtualMachine const&){};
	/** There can only be one mainJVM, this is a singleton class*/

	/** depending on which variable is set we will enter development mode */
	bool isDevelopmentMode;
	
	/** The constructed option string */
	char  			*libraryPathString;
    bool exceptionCheck(JNIEnv * env,int numCleanups,...);
    void cleanUpObjects(JNIEnv * env,int numCleanups,...);
    static jstring getSystemProperty(string propertyName);
    bool is64BitArchitecture();
    int majorOSVersion();
    
protected:

	bool launchJVM();

	//---------------------------------------------------------------------------------------------
	// Class members
	//---------------------------------------------------------------------------------------------

	// JNI-related variables
	JNIEnv          *env;
	JavaVM          *jvm;
	JavaVMInitArgs  vm_args;
	JavaVMOption    options[256];
	int             nbOptions;
	jint            res;
	jclass          myStartupJavaClass;
	jmethodID       constructorID;
	jobject JavaGUI;
	bool isLaunched;

	/** The java home directory as passed in at jvmstartup */
	char 			*javaHomeDirectory;

	/** The constructed classpath string */
	char            *classPathString;


	/** An array of additional options to pass along to the jvm */
	char			*additionalOptions[256];

	/** A list of library paths to feed the jvm */
	list<string> 	*libraryPaths;

	/** A list of class paths to feed the jvm */
	list<string> 	*classPaths;

	/** A list of additional options to feed the jvm */
	list<string> 	*jvmStartupOptions;

	/** The class loaded for startup the static main function will be called from the main c thread */
	string startUpClass;

	
	//our embedded library paths (future use)
	static string * embeddedJavaLibraryPath32;
    static string * embeddedJavaLibraryPath64;


    /** static instance of our singleton */
	static IVirtualMachine *instance;

private:
    
    
    /** we are a singleton so our cunstructor is private */
	IVirtualMachine();

#ifdef WIN32
	/** handle to the loaded JVM.DLL */
	HMODULE handle;
	_TCHAR* findLib();
	_TCHAR* checkVMRegistryKey(HKEY jreKey, _TCHAR* subKeyName);
	void logLastError(LPTSTR lpszFunction);
#else
	void * handle;
#endif
    
    
    
};






#endif /* IVIRTUALMACHINE_H_ */
