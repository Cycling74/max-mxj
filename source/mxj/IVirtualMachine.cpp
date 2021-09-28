/*
 * IVirtualMachine.cpp
 *
 *	Created on: Feb 4, 2012
 *	Modified for Cycling '74 October 2015
 *		Author: Peter J Slack, P.Eng
 *	Copyright 2012-2015 WaveDNA Ltd. All rights reserved
 *
 */

#ifdef WIN32
#include <cstdio>
#else
#include <stdio.h>
#endif
#include <assert.h>
#include "IVirtualMachine.h"

//here we need type definitions that will map in when jvm.dll (or dylib) is loaded
typedef _JNI_IMPORT_OR_EXPORT_ jint	 (*WRAPPED_JNI_CreateJavaVM)(JavaVM **pvm, void **penv, void *args);
typedef _JNI_IMPORT_OR_EXPORT_ jint	 (*WRAPPED_JNI_GetCreatedJavaVMs)(JavaVM **, jsize, jsize *);

#include <sys/types.h>
#include <sys/stat.h>



#ifdef WIN_VERSION
#include <stdio.h>
#include <strsafe.h>
#include <atlstr.h>

#else

#include <pthread.h>
#include <dlfcn.h>



// library handle for calling objc_registerThreadWithCollector()
// without static linking to the libobjc library
#define OBJC_LIB "/usr/lib/libobjc.dylib"
#define OBJC_GCREGISTER "objc_registerThreadWithCollector"
typedef void (*objc_registerThreadWithCollector_t)();
extern "C" objc_registerThreadWithCollector_t objc_registerThreadWithCollectorFunction;
objc_registerThreadWithCollector_t objc_registerThreadWithCollectorFunction = NULL;

extern "C"{
static bool awtLoaded = false;
static pthread_mutex_t awtLoaded_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  awtLoaded_cv = PTHREAD_COND_INITIALIZER;

JNIEXPORT void JNICALL
JLI_NotifyAWTLoaded()
{
	pthread_mutex_lock(&awtLoaded_mutex);
	awtLoaded = true;
	pthread_cond_signal(&awtLoaded_cv);
	pthread_mutex_unlock(&awtLoaded_mutex);
}
}

#endif

using namespace std;


//	When launching JVM on OSX we need to do so on a separate thread
//	This is the thread launch callback
void *JVMThread(void *param)
{

#ifdef MAC_VERSION
	pthread_setname_np("IVirtualMachine JVM Launch Thread");
#endif
	IVirtualMachine * myMachine = IVirtualMachine::getInstance();
	myMachine->startJVM();


	return NULL;
}


#ifdef WIN_VERSION
inline bool file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

#endif

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

	//originally windows only launch

	startJVM();


#ifdef MAC_VERSION

	//	  pthread_t jvmThread;
	//
	//	// Create a new pthread copying the stack size of the primordial pthread
	//	struct rlimit limit;
	//	size_t stack_size = 0;
	//	int rc = getrlimit(RLIMIT_STACK, &limit);
	//	if (rc == 0) {
	//		if (limit.rlim_cur != 0LL) {
	//			stack_size = (size_t)limit.rlim_cur;
	//		}
	//	}
	//
	//	// Set up and launch the jvm main thread
	//	pthread_attr_t attr;
	//	int attr_init_rval;
	//	int attr_setstate_rval;
	//	int pthread_create_rval;
	//
	//	attr_init_rval=pthread_attr_init(&attr);
	//
	//	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	//	attr_setstate_rval=pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	//
	//	if (stack_size > 0)
	//	{
	//		pthread_attr_setstacksize(&attr, stack_size);
	//	}
	//
	//
	//	pthread_create_rval=pthread_create(&jvmThread, &attr, &JVMThread, NULL);
	//	if(pthread_create_rval != 0){
	//		  //error condition
	//		  return;
	//	  }
	//
	//
	//	int pjrval = pthread_join(jvmThread,NULL);
	//
	//	  if(pjrval!=0)
	//	  {
	//		  //error condition
	//		  return;
	//	  }
	//
	//	  pthread_attr_destroy(&attr);



#endif

}




void IVirtualMachine::startJVM()
{


#ifdef WIN_VERSION

	// Find jvm dll lib path
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
			// Logger::writeToLog("Out of memory for environment block swap");
		}else{
			// Get path env
			dwRet = GetEnvironmentVariable(VARNAME, pszOldVal, ENV_BUFSIZE);

			if(dwRet == 0){
				//variable doesn't exist

			}else{
				//calculate the length of the updated path
				string jvmLibPath(eclipseLibrary);
				string newPath(jvmLibPath);
				newPath.append(string(";"));
				newPath.append(string(pszOldVal));

				if (! SetEnvironmentVariable(VARNAME, newPath.data()))
				{
					logLastError(TEXT("SetEnvironmentVariable failed after adding jvm lib"));
					//Logger::writeToLog("SetEnvironmentVariable failed");
				}

				//Logger::writeToLog(" new environment ");
				//Logger::writeToLog(oldPath);

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

	// dynamically link to objective c gc registration
	void *handleLibObjc = dlopen(OBJC_LIB, RTLD_LAZY);
	if (handleLibObjc != NULL) {
		objc_registerThreadWithCollectorFunction = (objc_registerThreadWithCollector_t) dlsym(handleLibObjc, OBJC_GCREGISTER);
	}


	char * baseDir = getJavaHome();
	char * dylib = findLib(baseDir);
	char * jli = getJavaJli();


	//we look to the environment for an embedded path
	char * embeddedEnvironment = getenv("EMBEDDED_JVM_LIBRARY_PATH");
	char * embeddedJliEnvironment = getenv("EMBEDDED_JLI_LIBRARY_PATH");

	if(embeddedEnvironment!=NULL)
		dylib = embeddedEnvironment;

	//embedded users must also provide the full path to the JLI library
	if(embeddedJliEnvironment!=NULL)
		jli = embeddedJliEnvironment;


	//the jvm can still launch if jli is not present
	//in the case of Apple java 1.6 for example this will not result in a problem
	if(jli!= NULL)
	{
		dlopen(jli, RTLD_NOW + RTLD_GLOBAL);
	}

	if(dylib!=NULL)
	{

		handle= dlopen(dylib,RTLD_NOW + RTLD_GLOBAL);
		if(handle==NULL)
		{
			return;
		}
	}
	else
		return;
#endif

	// launch the jvm
	launchJVM();

}


jstring IVirtualMachine::getSystemProperty(string propertyName)
{
	jstring rval = NULL;

	IVirtualMachine * myMachine = IVirtualMachine::getInstance();
	//let's try some awt launching stuff here
	JNIEnv* env =myMachine->attachJNIThread();
	if(env!=NULL)
	{
		jclass sysClass = env->FindClass("java/lang/System");
		if(!sysClass){
			myMachine->exceptionCheck(env, 0);
		}

		jmethodID sysGetProperty = env->GetStaticMethodID(sysClass, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
		if(!sysGetProperty){
			myMachine->exceptionCheck(env, 1,sysClass);
			return rval;
		}

		jstring propName = env->NewStringUTF(propertyName.c_str());


		rval = (jstring)env->CallStaticObjectMethod(sysClass, sysGetProperty,propName);



		if(myMachine->exceptionCheck(env, 2,sysClass,propName))
		{
			return NULL;
		}

		myMachine->cleanUpObjects(env, 2, propName, sysClass);
	}



	return rval;
}


#ifdef WIN_VERSION

// Path to JRE found previously
extern "C" {
const char *getGlobal_jrepath();
const char *getGlobal_jvmpath();
const char *getGlobal_jvmtype();
}

/*
 * Find the VM shared library starting from the java executable
 */
_TCHAR*	 IVirtualMachine::findLib() {

	int	 j;

	_TCHAR * path;				/* path to resulting jvm shared library */

	// Did we found a JRE before ? embeded of system installed
	const char *global_jvmpath = getGlobal_jvmpath();
	if (global_jvmpath != NULL && *global_jvmpath != 0)
	{
		size_t newsize = strlen(global_jvmpath) + 1;
		path = new _TCHAR[newsize];
		_tcscpy(path, A2T((char*)global_jvmpath));
		return path;
	}

	/* for looking in the registry */
	HKEY jreKey = NULL;
	DWORD length = MAX_PATH;
	_TCHAR keyName[MAX_PATH];
	_TCHAR * jreKeyName;		
	
	/* Not found yet, try the registry, we will use the first vm >= 1.4 */
	jreKeyName = _T("Software\\JavaSoft\\Java Runtime Environment");
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


	if(RegOpenKeyEx(jreKey, subKeyName, 0, KEY_READ, &subKey) == ERROR_SUCCESS) {
		/*The RuntimeLib value should point to the library we want*/
		if(RegQueryValueEx(subKey, _T("RuntimeLib"), NULL, NULL, (LPBYTE)&value, &length) == ERROR_SUCCESS) {

			//so we have a glitch in certain JVM installs version 1.7+
			//the regstry says the files are on ../client
			// but in fact they are at ../server.	 so we do a dance here to make the extra check
			//in the case that we get a reg value but the file doesn't exist

			string myLib (value);
			const string replacement("server");
			if(!file_exists(myLib)){
				size_t ps = myLib.find(string("client"), 0);
				if (ps != string::npos){


					myLib.replace(ps, 6, replacement);


					if (file_exists(myLib)){
						memcpy(value, myLib.data(), myLib.length());
						result = _tcsdup(value);
					}
				}
				else
				{
					result = (_TCHAR *)NULL;
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
	size_t j = 0;

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

	vm_args.version = JNI_VERSION_1_6;					 /* Specifies the JNI version used */

	vm_args.options	 = options;
	vm_args.nOptions = nbOptions;
	vm_args.ignoreUnrecognized = JNI_TRUE;				   /* JNI won't complain about unrecognized options */

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




void _append_exception_trace_messages(
									  JNIEnv&	   a_jni_env,
									  std::string& a_error_msg,
									  jthrowable   a_exception,
									  jmethodID	   a_mid_throwable_getCause,
									  jmethodID	   a_mid_throwable_getStackTrace,
									  jmethodID	   a_mid_throwable_toString,
									  jmethodID	   a_mid_frame_toString)
{
	// Get the array of StackTraceElements.
	jobjectArray frames =
	(jobjectArray) a_jni_env.CallObjectMethod(
											  a_exception,
											  a_mid_throwable_getStackTrace);
	jsize frames_length = a_jni_env.GetArrayLength(frames);

	// Add Throwable.toString() before descending
	// stack trace messages.
	if (0 != frames)
	{
		jstring msg_obj =
		(jstring) a_jni_env.CallObjectMethod(a_exception,
											 a_mid_throwable_toString);
		const char* msg_str = a_jni_env.GetStringUTFChars(msg_obj, 0);

		// If this is not the top-of-the-trace then
		// this is a cause.
		if (!a_error_msg.empty())
		{
			a_error_msg += "\nCaused by: ";
			a_error_msg += msg_str;
		}
		else
		{
			a_error_msg = msg_str;
		}

		a_jni_env.ReleaseStringUTFChars(msg_obj, msg_str);
		a_jni_env.DeleteLocalRef(msg_obj);
	}

	// Append stack trace messages if there are any.
	if (frames_length > 0)
	{
		jsize i = 0;
		for (i = 0; i < frames_length; i++)
		{
			// Get the string returned from the 'toString()'
			// method of the next frame and append it to
			// the error message.
			jobject frame = a_jni_env.GetObjectArrayElement(frames, i);
			jstring msg_obj =
			(jstring) a_jni_env.CallObjectMethod(frame,
												 a_mid_frame_toString);

			const char* msg_str = a_jni_env.GetStringUTFChars(msg_obj, 0);

			a_error_msg += "\n	  ";
			a_error_msg += msg_str;

			a_jni_env.ReleaseStringUTFChars(msg_obj, msg_str);
			a_jni_env.DeleteLocalRef(msg_obj);
			a_jni_env.DeleteLocalRef(frame);
		}
	}

	// If 'a_exception' has a cause then append the
	// stack trace messages from the cause.
	if (0 != frames)
	{
		jthrowable cause =
		(jthrowable) a_jni_env.CallObjectMethod(
												a_exception,
												a_mid_throwable_getCause);
		if (0 != cause)
		{
			_append_exception_trace_messages(a_jni_env,
											 a_error_msg,
											 cause,
											 a_mid_throwable_getCause,
											 a_mid_throwable_getStackTrace,
											 a_mid_throwable_toString,
											 a_mid_frame_toString);
		}
	}
}


/** Error reporting with jboject cleanup)
 pass any jobjects that need to be cleaned out on error
 @parameter env - the JNI Environment
 @parameter numCleanups - the number of open objects to delete
 @parameter ... - cvariable list of objects to delete local reference
 returns true if errors were encountered
 */
bool IVirtualMachine::exceptionCheck(JNIEnv * env,int numCleanups,...)
{
	// Get the exception and clear as no
	// JNI calls can be made while an exception exists.
	jthrowable exception = env->ExceptionOccurred();
	if (exception != NULL)
	{

		env->ExceptionClear();

		jclass throwable_class = env->FindClass("java/lang/Throwable");
		jmethodID mid_throwable_getCause =
		env->GetMethodID(throwable_class,
						 "getCause",
						 "()Ljava/lang/Throwable;");
		jmethodID mid_throwable_getStackTrace =
		env->GetMethodID(throwable_class,
						 "getStackTrace",
						 "()[Ljava/lang/StackTraceElement;");
		jmethodID mid_throwable_toString =
		env->GetMethodID(throwable_class,
						 "toString",
						 "()Ljava/lang/String;");

		jclass frame_class = env->FindClass("java/lang/StackTraceElement");
		jmethodID mid_frame_toString =
		env->GetMethodID(frame_class,
						 "toString",
						 "()Ljava/lang/String;");


		std::string error_msg; // Could use ostringstream instead.

		_append_exception_trace_messages(*env,
										 error_msg,
										 exception,
										 mid_throwable_getCause,
										 mid_throwable_getStackTrace,
										 mid_throwable_toString,
										 mid_frame_toString);
		cout << error_msg;

		va_list ap;
		va_start(ap, numCleanups);
		for(int i=0;i<numCleanups;i++)
		{
			env->DeleteLocalRef(va_arg(ap, jobject));
			if(exceptionCheck(env,0))
			{
				cout << "NFG on Cleanup";
			}
		}
		va_end(ap);

		return true;


	}


	return false;
}

/**
 cleans up jobjects in long jni sequences
 @parameter env - the JNI Environment
 @parameter numCleanups - the number of open objects to delete
 @parameter ... - cvariable list of objects to delete local reference

 */
void IVirtualMachine::cleanUpObjects(JNIEnv * env,int numCleanups,...)
{
	va_list ap;
	va_start(ap, numCleanups);
	for(int i=0;i<numCleanups;i++)
	{
		env->DeleteLocalRef(va_arg(ap, jobject));
		if(exceptionCheck(env,0))
		{
			//DBG("NFG on Cleanup");
		}
	}
	va_end(ap);

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
jstring get_system_property(ivirtualmachine* v,char * property)
{
	string myProp(property);
	return real(v)->getSystemProperty(myProp);
}
bool is64BitArchitecture(ivirtualmachine* v){return real(v)->is64BitArchitecture();}
int getMajorOSVersion(ivirtualmachine* v){return real(v)->majorOSVersion();}
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

	//Logger::writeToLog(String((LPCTSTR)lpDisplayBuf));

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

#endif


bool IVirtualMachine::is64BitArchitecture()
{
	bool retval = false;
#ifdef MAC_VERSION
	OSXSys q;
	retval = q.is64BitRunning();
#endif
	return retval;
}
int IVirtualMachine::majorOSVersion()
{

	int retval = 0 ;
#ifdef MAC_VERSION
	OSXSys q;
	retval = q.getMajorOSVersion();
#endif
	return retval;
}


