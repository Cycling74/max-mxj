/*
 * classes.c -- operations on Java classes.
 */

#ifdef MAC_VERSION
#include <JavaVM/jni.h>        // Java Native Interface definitions
#include <JavaVM/jni_md.h>
#else
#include "jni.h"
#include "jni_md.h"
#endif
#include "classes.h"
#include "ExceptionUtils.h"

static jclass    s_loader_class     = 0;	 
static jmethodID s_get_instance_mid = 0;
static jmethodID s_load_class_mid   = 0; 
static jmethodID s_add_dir_mid   = 0;
static jmethodID s_dump_mid   = 0;
static jmethodID s_set_ext_class_search_mid = 0;


#define JNI_PUSH_LOCAL_FRAME MXJ_JNI_CALL(env,PushLocalFrame)(env,8);
#define JNI_POP_LOCAL_FRAME MXJ_JNI_CALL(env,PopLocalFrame)(env,NULL);

short init_mxj_classloader(JNIEnv *env);	
short init_mxj_classloader(JNIEnv *env)
{	
	int err = 0;	
	
	JNI_PUSH_LOCAL_FRAME
	
	s_loader_class 	   = MXJ_JNI_CALL(env,FindClass)(env,"com/cycling74/max/MXJClassLoader");
	s_loader_class     = MXJ_JNI_CALL(env,NewGlobalRef)(env,s_loader_class);
	err = checkException(env);
	if(err)return -10;
	s_get_instance_mid   = MXJ_JNI_CALL(env,GetStaticMethodID)(env, s_loader_class,"getInstance", "()Lcom/cycling74/max/MXJClassLoader;");
	err = checkException(env);
	if(err)return -20;
	s_load_class_mid     = MXJ_JNI_CALL(env,GetMethodID)(env, s_loader_class,"loadClazz", "(Ljava/lang/String;ZZ)Ljava/lang/Class;");
	err = checkException(env);
	if(err)return -30;
	s_add_dir_mid   =  MXJ_JNI_CALL(env,GetMethodID)(env, s_loader_class,"addDirectory", "(Ljava/lang/String;)V");
	err = checkException(env);
	if(err)return -50;
	s_dump_mid   =  MXJ_JNI_CALL(env,GetMethodID)(env, s_loader_class,"dump", "()V");
	err = checkException(env);
	if(err)return -60;	
	s_set_ext_class_search_mid = MXJ_JNI_CALL(env,GetMethodID)(env, s_loader_class,"setExtendedClassSearchDirectory", "(Ljava/lang/String;)V");
	err = checkException(env);
	if(err)return -70;	
	
	JNI_POP_LOCAL_FRAME
	
	return 0;
}


void mxj_classloader_add_dir(JNIEnv *env, char *dirname)
{
	jobject mxj_classloader = 0;
	jstring jdirname = 0;
	int err = 0;
	
	JNI_PUSH_LOCAL_FRAME
	
	if(s_loader_class == 0) {
		err = init_mxj_classloader(env);
		if (err) {
	 		error("(mxj add dir) unable to init mxj classloader. ERROR: %d",err);
	 		return;	
		}
	}
	
	mxj_classloader = MXJ_JNI_CALL(env, CallStaticObjectMethod)(env, s_loader_class, s_get_instance_mid);
	checkException(env);
 	jdirname = MXJ_JNI_CALL(env, NewStringUTF)(env, dirname);
 	checkException(env);
 	MXJ_JNI_CALL(env, CallVoidMethod)(env, mxj_classloader, s_add_dir_mid, jdirname);
	checkException(env);
	
	JNI_POP_LOCAL_FRAME
}


void mxj_classloader_set_extended_classsearch_dir(JNIEnv *env, char *dirname)
{
	jobject mxj_classloader = 0;
	jstring jdirname = 0;
	int err = 0;
	
	JNI_PUSH_LOCAL_FRAME
	
	if(s_loader_class == 0) {
		err = init_mxj_classloader(env);
		if(err) {
	 		error("(mxj add dir) unable to init mxj classloader. ERROR: %d",err);
	 		return;	
		}
	}
	
	mxj_classloader = MXJ_JNI_CALL(env,CallStaticObjectMethod)(env,s_loader_class,s_get_instance_mid);
	checkException(env);
 	jdirname = MXJ_JNI_CALL(env,NewStringUTF)(env,dirname);
 	checkException(env);
 	MXJ_JNI_CALL(env,CallVoidMethod)(env,mxj_classloader,s_set_ext_class_search_mid,jdirname);
	checkException(env);
	
	JNI_POP_LOCAL_FRAME
}

void mxj_classloader_unset_extended_classsearch_dir(JNIEnv *env)
{
	jobject mxj_classloader = 0;
	int err = 0;
	
	JNI_PUSH_LOCAL_FRAME
	
	if(s_loader_class == 0) {
		err = init_mxj_classloader(env);
		if(err){
			error("(mxj add dir) unable to init mxj classloader. ERROR: %d",err);
	 		return;	
		}
	}
	
	mxj_classloader = MXJ_JNI_CALL(env,CallStaticObjectMethod)(env,s_loader_class,s_get_instance_mid);
	checkException(env);
 	MXJ_JNI_CALL(env,CallVoidMethod)(env,mxj_classloader,s_set_ext_class_search_mid,NULL);
	checkException(env);
	
	JNI_POP_LOCAL_FRAME
}

void mxj_classloader_dump(JNIEnv *env)
{
	jobject mxj_classloader = 0;
	int err = 0;
	
	JNI_PUSH_LOCAL_FRAME
	if(s_loader_class == 0) {
		err = init_mxj_classloader(env);
		if(err) {
			error("(mxj classloader dump) unable to init mxj classloader. ERROR: %d",err);
			return;
		}
	}
	
	mxj_classloader = MXJ_JNI_CALL(env,CallStaticObjectMethod)(env,s_loader_class,s_get_instance_mid);
	checkException(env);
 	MXJ_JNI_CALL(env,CallVoidMethod)(env,mxj_classloader,s_dump_mid);
 	checkException(env);
	
	JNI_POP_LOCAL_FRAME
}


t_max_err mxj_classloader_load_class(JNIEnv *env, char *className, jclass *cls)
{
	short err;
	jobject mxj_classloader = 0;
	jclass l_class = NULL;
	jboolean resolve = true;
	jboolean report_error = true;
	jstring jclassname = NULL;
	
	JNI_PUSH_LOCAL_FRAME
	
	if(s_loader_class == 0) {
		err = init_mxj_classloader(env);
		if(err) {
			error("unable to init mxj classloader. ERROR: %d",err);
	  		error("Could not load class '%s'....unable to init_mxj_classloader", className);
			return MAX_ERR_GENERIC;
		}
	}
	
	mxj_classloader = MXJ_JNI_CALL(env,CallStaticObjectMethod)(env,s_loader_class,s_get_instance_mid);
	checkException(env);
	jclassname = MXJ_JNI_CALL(env,NewStringUTF)(env,className);
	checkException(env);	
	l_class = MXJ_JNI_CALL(env,CallObjectMethod)(env,mxj_classloader,s_load_class_mid,jclassname,resolve,report_error);
	
	if(checkException(env) || !l_class ) {
		error("Could not load class '%s'", className);
		return MAX_ERR_GENERIC;
	}

    *cls = MXJ_JNI_CALL(env,NewGlobalRef)(env,l_class);
	
	JNI_POP_LOCAL_FRAME
	
    return MAX_ERR_NONE;
}	

/*
 * Get a class by name.
 */
jclass getClassByName(JNIEnv *env, char *className) {

	jclass l_class = NULL;
	jclass g_class = NULL;
	
	JNI_PUSH_LOCAL_FRAME
	
	MXJ_JNI_CALL(env,ExceptionClear)(env);	
	l_class = MXJ_JNI_CALL(env,FindClass)(env, className);
	
	if (MXJ_JNI_CALL(env,ExceptionCheck)(env) || (l_class == NULL)) {
		MXJ_JNI_CALL(env,ExceptionDescribe)(env);
		MXJ_JNI_CALL(env,ExceptionClear)(env);
		error("Could not load class '%s'", className);
		return 0;
		//not in class path here we could call out own stuff to find class
		//-tml	
	}

	g_class = MXJ_JNI_CALL(env,NewGlobalRef)(env, l_class);

    if (g_class == NULL) {
    	error("Out of memory creating global ref for class '%s'", className);
		return 0;
    }
    
    JNI_POP_LOCAL_FRAME
    
	return g_class;
}
