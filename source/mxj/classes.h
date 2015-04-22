/*
 * Information related to the Java classes we use.
 */
#include "package.h"

#ifndef _Included_classes_h
#define _Included_classes_h


#ifdef MXJ_MSP
	#define MAX_CLASSNAME "mxj~"
	#define MAX_FRAMEWORK_CLASSNAME	"MSPObject"
#else
	#define MAX_CLASSNAME "mxj"
	#define MAX_FRAMEWORK_CLASSNAME "MaxObject"
#endif
	

#ifdef MXJ_MSP    
	#define MAX_FRAMEWORK_CLASS "com/cycling74/msp/MSPObject"
#else
	#define MAX_FRAMEWORK_CLASS MAX_FRAMEWORK_PKG"/"MAX_FRAMEWORK_CLASSNAME
#endif

#define STRING_CLASSNAME "java/lang/String"

// The maximum length allowed for a classname
#define MAX_CLASSNAME_LENGTH 512

jclass getClassByName(JNIEnv *env, char *className);

//load a class with the mxj classloader java class
t_max_err mxj_classloader_load_class(JNIEnv *env, char *classname, jclass *cls);
void mxj_classloader_add_dir(JNIEnv *env, char *dirname);
void mxj_classloader_zap(JNIEnv *env);
void mxj_classloader_dump(JNIEnv *env);
void mxj_classloader_set_extended_classsearch_dir(JNIEnv *env, char *dirname);
void mxj_classloader_unset_extended_classsearch_dir(JNIEnv *env);
#endif
