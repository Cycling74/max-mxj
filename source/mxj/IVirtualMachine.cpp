/*
 * IVirtualMachine.cpp
 *
 *  Created on: Feb 4, 2012
 *  Modified for Cycling '74 October 2015
 *      Author: Peter J Slack, P.Eng
 *  Copyright 2012-2015 WaveDNA Ltd. All rights reserved
 *
 */

#include <cstdio>
#include <assert.h>
#include "IVirtualMachine.h"

//here we need type definitions that will map in when jvm.dll (or dylib) is loaded
typedef _JNI_IMPORT_OR_EXPORT_ jint  (*WRAPPED_JNI_CreateJavaVM)(JavaVM **pvm, void **penv, void *args);
typedef _JNI_IMPORT_OR_EXPORT_ jint  (*WRAPPED_JNI_GetCreatedJavaVMs)(JavaVM **, jsize, jsize *);

#include <sys/types.h>
#include <dirent.h>



#ifdef WIN_VERSION
#include <stdio.h>
#include <strsafe.h>
#include <sys/stat.h>

#else

#include <pthread.h>
#include <dlfcn.h>


#endif

using namespace std;

//  When launching JVM on OSX we need to do so on a separate thread
//  This is the thread launch callback
void *JVMThread(void *param)
{

#ifdef MAC_VERSION
	pthread_setname_np("IVirtualMachine JVM Launch Thread");
#endif

	(IVirtualMachine::getInstance())->startJVM();
    
	return NULL;
}


//-------------------------------------------------------------------------------------------------
// Initialize static class members
//-------------------------------------------------------------------------------------------------

IVirtualMachine *IVirtualMachine::instance = NULL;

//currently not used, these will allow us to adpot an embedded strategy
//so product developers can distribute apps without having the user to install java
string *IVirtualMachine::embeddedJavaLibraryPath32 = NULL;
string *IVirtualMachine::embeddedJavaLibraryPath64 = NULL;

IVirtualMachine::IVirtualMachine()
{
	isLaunched=false;
	isDevelopmentMode=false;
	
	libraryPathString=NULL;
	libraryPaths=new list<string>();
	classPaths=new list<string>();
	jvmStartupOptions = new list<string>();
	jvm = NULL;
	env = NULL;
#ifdef WIN_VERSION
	handle = NULL;
#endif
}



void IVirtualMachine::startJava()
{


#ifdef WIN_VERSION
	startJVM();
#endif

	
#ifdef MAC_VERSION

    pthread_t jvmThread;

	// Create a new pthread copying the stack size of the primordial pthread
	struct rlimit limit;
	size_t stack_size = 0;
	int rc = getrlimit(RLIMIT_STACK, &limit);
	if (rc == 0) {
		if (limit.rlim_cur != 0LL) {
			stack_size = (size_t)limit.rlim_cur;
		}
	}

	// Set up and launch the jvm main thread
	pthread_attr_t attr;
	int attr_init_rval;
	int attr_setstate_rval;
	int pthread_create_rval;

	attr_init_rval=pthread_attr_init(&attr);

	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	attr_setstate_rval=pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if (stack_size > 0)
	{
		pthread_attr_setstacksize(&attr, stack_size);
	}


	pthread_create_rval=pthread_create(&jvmThread, &attr, &JVMThread, NULL);
	if(pthread_create_rval != 0){
        //error condition
    }

	pthread_attr_destroy(&attr);

	pthread_join(jvmThread,NULL);



#endif

}




void IVirtualMachine::startJVM()
{
    
    
#ifdef WIN_VERSION
    
    _TCHAR * eclipseLibrary = findLib();
    
    if(eclipseLibrary != NULL){
        
        //if we made it this far let's adjust the environment to fix for
        // path problem issues any errors in this block we will simply not do the environment swap
        
#define ENV_BUFSIZE 32767
#define VARNAME TEXT("Path")
        
        DWORD dwRet;
        LPTSTR pszOldVal;
        
        pszOldVal = (LPTSTR) malloc(ENV_BUFSIZE*sizeof(TCHAR));
        
        if(NULL == pszOldVal)
        {
            Logger::writeToLog("Out of memory for environment block swap");
        }else{
            
            dwRet = GetEnvironmentVariable(VARNAME, pszOldVal, ENV_BUFSIZE);
            
            if(dwRet == 0){
                //variable doesn't exist
                
            }else{
                //calculate the length of the updated path
                File jvmLib = File(eclipseLibrary);
                String jvmLibPath = jvmLib.getParentDirectory().getParentDirectory().getFullPathName();
                String oldPath = String(pszOldVal);
                oldPath.append(String(";") + jvmLibPath , ENV_BUFSIZE - oldPath.length());
                
                if (! SetEnvironmentVariable(VARNAME, oldPath.getCharPointer()))
                {
                    Logger::writeToLog("SetEnvironmentVariable failed");
                }
                
                Logger::writeToLog(" new environment ");
                Logger::writeToLog(oldPath);
                
            }
            
            free(pszOldVal);
            
        }
        
        handle = LoadLibrary(eclipseLibrary);
        if(handle == NULL){
            logLastError(TEXT("startJVM - loadlibrary call"));
            return;
        }
        
    }else{
        return;
    }
    
#endif
    
#ifdef MAC_VERSION
    
    char * baseDir = getJavaHome();
    char * dylib = findLib(baseDir);
    
    if(dylib!=NULL)
        handle= dlopen(dylib,RTLD_NOW);
    else
        return;
#endif
    
    // launch the jvm
    launchJVM();
    
}




#ifdef WIN_VERSION
/*
 * Find the VM shared library starting from the java executable 
 */
_TCHAR*  IVirtualMachine::findLib() {

	int i, j;
	size_t pathLength;	
	struct _stat stats;
	_TCHAR * path;				/* path to resulting jvm shared library */
	_TCHAR * location;			/* points to begining of jvmLocations section of path */
	
	/* for looking in the registry */
	HKEY jreKey = NULL;
	DWORD length = MAX_PATH;
	_TCHAR keyName[MAX_PATH];
	_TCHAR * jreKeyName;		
	
	/* Not found yet, try the registry, we will use the first vm >= 1.4 */
	//here we use the path to 32 bit versions in wow6432Node key path
	//this will change when we look at 64 bitness
#ifdef X64
	jreKeyName = _T("Software\\JavaSoft\\Java Runtime Environment");
#else
	BOOL bIsWow64 = FALSE;
	IsWow64Process(GetCurrentProcess(),&bIsWow64);
	if(bIsWow64){
		jreKeyName = _T("Software\\Wow6432Node\\JavaSoft\\Java Runtime Environment");
	}else{
		jreKeyName = _T("Software\\JavaSoft\\Java Runtime Environment");	
	}
#endif
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, jreKeyName, 0, KEY_READ, &jreKey) == ERROR_SUCCESS) {
		if(RegQueryValueEx(jreKey, _T("CurrentVersion"), NULL, NULL, (LPBYTE)&keyName, &length) == ERROR_SUCCESS) {
			path = checkVMRegistryKey(jreKey, keyName);
			if (path != NULL) {
				RegCloseKey(jreKey);
				return path;
			}
		}
		j = 0;
		length = MAX_PATH;
		while (RegEnumKeyEx(jreKey, j++, keyName, &length, 0, 0, 0, 0) == ERROR_SUCCESS) {  
			/*look for a 1.6 vm*/ 
			if( _tcsncmp(_T("1.6"), keyName, 3) <= 0 ) {
				path = checkVMRegistryKey(jreKey, keyName);
				if (path != NULL) {
					RegCloseKey(jreKey);
					return path;
				}
			}
		}
		RegCloseKey(jreKey);
	}
	return NULL;
}

/*
 * Read the subKeyName subKey of jreKey and look to see if it has a Value 
 * "RuntimeLib" which points to a jvm library we can use 
 * 
 * Does not close jreKey
 */
 _TCHAR*  IVirtualMachine::checkVMRegistryKey(HKEY jreKey, _TCHAR* subKeyName) {
	_TCHAR value[MAX_PATH];
	HKEY subKey = NULL;
	DWORD length = MAX_PATH;
	_TCHAR *result = NULL;
	struct _stat stats;
	
	if(RegOpenKeyEx(jreKey, subKeyName, 0, KEY_READ, &subKey) == ERROR_SUCCESS) {				
		/*The RuntimeLib value should point to the library we want*/
		if(RegQueryValueEx(subKey, _T("RuntimeLib"), NULL, NULL, (LPBYTE)&value, &length) == ERROR_SUCCESS) {

	  //so we have a glitch in certain JVM installs version 1.7+
      //the regstry says the files are on ../client
	  // but in fact they are at ../server.  so we do a dance here to make the extra check
	  //in the case that we get a reg value but the file doesn't exist

			String myLib = String(value);
			File myLibFile = File(myLib);
			if(!myLibFile.exists()){
			  String newLib = myLib.replace("client","server");
			  File myNewLibFile = File(newLib);
			  if(myNewLibFile.exists()){
				  memcpy(value,newLib.getCharPointer(), newLib.length());
				  result = _tcsdup(value);
			  }
			} else{
				result = _tcsdup(value);
			}
	

		}
		RegCloseKey(subKey);
	}
	return result;
}

#endif

void IVirtualMachine::killJVM()
{
	if (jvm != NULL)
	{
		jvm->DestroyJavaVM();
	}
}



IVirtualMachine::~IVirtualMachine()
{
	libraryPaths->clear();
	classPaths->clear();
	jvmStartupOptions->clear();
	delete (libraryPaths);
	delete (classPaths);
	delete (jvmStartupOptions);


	if(IVirtualMachine::embeddedJavaLibraryPath32 != NULL){
		delete(IVirtualMachine::embeddedJavaLibraryPath32);
		IVirtualMachine::embeddedJavaLibraryPath32 = NULL;
	}
	if(IVirtualMachine::embeddedJavaLibraryPath64 != NULL){
		delete(IVirtualMachine::embeddedJavaLibraryPath64);
		IVirtualMachine::embeddedJavaLibraryPath64 = NULL;
	}

	// Free the option strings provided to the jvm
	if(isLaunched)
	{
		if (classPathString != NULL)
		{
			free(classPathString);
		}

		if (libraryPathString != NULL)
		{
			free(libraryPathString);
		}

		for(int q=2;q<nbOptions;q++)
		{
			free (options[q].optionString);
		}

		if (jvm != NULL)
		{
			jvm = NULL;
		}
	}

	instance=NULL;

#ifdef WIN_VERSION
	//free the jvm.dll library when we die
	if(handle != NULL){
		FreeLibrary(handle);
	}
#endif

#ifdef MAC_VERSION
#endif

}

void IVirtualMachine::setStartupClass(string startClass)
{
	this->startUpClass=startClass;
}

void IVirtualMachine::addClassPath(string newPath)
{
	if(!newPath.empty())
		this->classPaths->push_front(newPath);
}

void IVirtualMachine::addLibraryPath(string newPath)
{
	if(!newPath.empty())
		this->libraryPaths->push_front(newPath);
}

void IVirtualMachine::addJavaOption(string newOption)
{
	if(!newOption.empty())
		this->jvmStartupOptions->push_front(newOption);
}

//-------------------------------------------------------------------------------------------------
// Private methods
//-------------------------------------------------------------------------------------------------

/**
 * This function launches the jvm given the root of the JRE
 * inputs- string homeDirectory - the absolute JAVA_HOME path
 */
bool IVirtualMachine::launchJVM()
{
    int j = 0;
    
    if(this->isLaunched)
        return false;
    
    // This is an error we should deallocate and return false for jvm launch
    if(jvmStartupOptions->size() > 256)
    {
        return false;
    }
    
    // First let us construct a list of classPaths
    string *constClassPath = new string("-Djava.class.path=");
    
    list<string>::iterator i;
    
    for(i=classPaths->begin(), j = 0; i != this->classPaths->end(); ++i, ++j)
    {
        *constClassPath += *i;
        if (j < (classPaths->size() - 1))
        {
#ifdef WIN_VERSION
            *constClassPath += ";";
#else
            *constClassPath += ":";
#endif
        }
    }
    
    classPathString=(char *)malloc(constClassPath->size()+1);
    strcpy(classPathString, constClassPath->data());
    
    // Construct a list of library paths
    //we never use this so for now it is not library path
    //it should report to bootclasspath
    string *constLibraryPath = new string("-Djava.class.path=");
    
    for(i=libraryPaths->begin(), j = 0; i != libraryPaths->end(); ++i, ++j)
    {
        *constLibraryPath +=  *i;
        if (j < (libraryPaths->size() - 1))
        {
            
#ifdef WIN32
            *constLibraryPath += ";";
#else
            *constLibraryPath += ":";
#endif
            
        }
    }
    
    libraryPathString=(char *)malloc(constLibraryPath->size()+1);
    strcpy(libraryPathString, constLibraryPath->data());
    
    
    nbOptions=0;
    options[nbOptions].optionString = libraryPathString;
    this->additionalOptions[nbOptions] = libraryPathString;
    nbOptions++;
    
    for(i=jvmStartupOptions->begin(); i != jvmStartupOptions->end(); ++i)
    {
        this->additionalOptions[nbOptions] = (char *)malloc((*i).size()+1);
        strcpy(additionalOptions[nbOptions], (*i).data());
        options[nbOptions].optionString = additionalOptions[nbOptions];
        nbOptions++;
    }
    
    vm_args.version = JNI_VERSION_1_6;                   /* Specifies the JNI version used */
    
    vm_args.options  = options;
    vm_args.nOptions = nbOptions;
    vm_args.ignoreUnrecognized = JNI_TRUE;                 /* JNI won't complain about unrecognized options */
    
    JavaVM *vmBuffer[1] = {NULL};
    jsize nVMs = 0;
    
#ifdef WIN_VERSION
    // here we need to link our external jvm.dll calls to the loaded
    // library
    WRAPPED_JNI_CreateJavaVM my_JNI_CreateJavaVM = (WRAPPED_JNI_CreateJavaVM)GetProcAddress(handle,"JNI_CreateJavaVM");
    WRAPPED_JNI_GetCreatedJavaVMs my_JNI_GetCreatedJavaVMs = (WRAPPED_JNI_GetCreatedJavaVMs)GetProcAddress(handle,"JNI_GetCreatedJavaVMs");
    
#endif

#ifdef MAC_VERSION

    //first see if the apple naming is active
    WRAPPED_JNI_CreateJavaVM my_JNI_CreateJavaVM = (WRAPPED_JNI_CreateJavaVM) dlsym(handle,"JNI_CreateJavaVM_Impl");
    
    //we need to try the possibility that this is not Apple JVM where they mangle the names by appending _impl
    if(my_JNI_CreateJavaVM == NULL){
        my_JNI_CreateJavaVM = (WRAPPED_JNI_CreateJavaVM) dlsym(handle,"JNI_CreateJavaVM");
    }
    
    WRAPPED_JNI_GetCreatedJavaVMs my_JNI_GetCreatedJavaVMs = (WRAPPED_JNI_GetCreatedJavaVMs)dlsym(handle,"JNI_GetCreatedJavaVMs_Impl");
    
    if(my_JNI_GetCreatedJavaVMs == NULL){
        my_JNI_GetCreatedJavaVMs = (WRAPPED_JNI_GetCreatedJavaVMs)dlsym(handle,"JNI_GetCreatedJavaVMs");
    }
#endif
    
    //catch the condition here that we really can't map our main functions
    if(my_JNI_GetCreatedJavaVMs == NULL){
        return false;
    }
    
    res = my_JNI_GetCreatedJavaVMs(vmBuffer, 10, &nVMs);
    
    
    if (nVMs > 0)
    {
        jvm = vmBuffer[nVMs-1];
        res = jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (res == JNI_EDETACHED)
        {
            res = jvm->AttachCurrentThread((void**)&env, NULL);
            if (res<0)
            {
                //error condition can't attach to thread
            }
        }
    }
    else
    {
        if(my_JNI_CreateJavaVM == NULL){
            return false;
        }
        
        res = my_JNI_CreateJavaVM(&jvm, (void **)&env, &vm_args);
        
    }
    
    
    if(res == 0)
        this->isLaunched=true;
    
    if(!startUpClass.empty())
    {
        myStartupJavaClass = env->FindClass(startUpClass.data());
        if(myStartupJavaClass != NULL){
            constructorID = env->GetStaticMethodID(myStartupJavaClass, "main", "([Ljava/lang/String;)V");
            env->CallStaticVoidMethod(myStartupJavaClass, constructorID, NULL);
        }
    }
    
    return true;
}




IVirtualMachine * IVirtualMachine::getInstance()
{

	if (instance == NULL)
	{
		instance = new IVirtualMachine();
	}

    return instance;
}

JNIEnv * IVirtualMachine::attachJNIThread()
{
    JNIEnv* env;
    
    if (jvm != NULL) {
        
        if (jvm->AttachCurrentThread((void**)&env, NULL)<0){
            return (JNIEnv *) NULL;
        } else {
            return env;
        }
        
    }else{
        
        return (JNIEnv *) NULL;
        
    }
}


// These are c to c++ API, making our IVirtualMAchine functions available to MXJ code

inline IVirtualMachine* real(ivirtualmachine* d) { return static_cast<IVirtualMachine*>(d); }
extern "C" {
    ivirtualmachine* new_virtualmachine(){return IVirtualMachine::getInstance();}
    void delete_virtualmachine(ivirtualmachine* v){delete real(v);}
    void start_java(ivirtualmachine* v){(real(v))->startJava();}
    void add_java_option(ivirtualmachine* v,char * option){(real(v))->addJavaOption(string(option));}
    void add_java_classpath(ivirtualmachine* v,char * classpath){(real(v))->addClassPath(string(classpath));}
    JavaVM * get_java_vm(ivirtualmachine* v){return real(v)->getJVM();}
    JNIEnv * get_thread_env(ivirtualmachine* v){return real(v)->attachJNIThread();}

}

#ifdef WIN_VERSION
/////////////////////////////////////////////
//A utility function to get windows errors

void IVirtualMachine::logLastError(LPTSTR lpszFunction){
	LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 

	Logger::writeToLog(String((LPCTSTR)lpDisplayBuf));

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

#endif