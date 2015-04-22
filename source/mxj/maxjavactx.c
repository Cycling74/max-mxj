/*
 * maxjavactx.c - native methods for calling Max from Java.
 *
 * Methods implemented:
 *
 * static void doPost(byte message[])
 * static void doOuch(byte message[])
 * static void doError(byte message[])
 * static short doGetMaxVersion()
 * void doOutlet(int outletIdx, int value)
 * void doOutlet(int outletIdx, float value)
 * void doOutlet(int outletIdx, String value)
 * void doOutlet(int outletIdx, Atom value[])
 *
 * Author: Herb Jellinek
 */

// contains the external object's link to available Max functions
#include "ext.h"

// Java Native Interface
#include "jni.h"
#include "jni_md.h"

#include "package.h"

#include "maxjava.h"
#include "callbacks.h"
#include "classes.h"

#pragma export on
#include "com_cycling74_max_MaxObject.h"  // prototypes of the Java methods we'll call
#pragma export reset

// Get the callbacks from the class.
VoidFnPtr *getCallbacks(JNIEnv *env, jclass maxObjectClass);

// Get a class by name.
jclass getClassByName(JNIEnv *env, char *className);

/*
 * main method: needed for linking, but is not executed
 */
void ext_main(void *r) {
}

/*
 * Call error on the message.  (Static method.)
 */
JNIEXPORT void JNICALL Java_com_cycling74_max_MaxObject_doError(
		JNIEnv *env,
		jclass cls,
		jbyteArray bytes) {
		
	VoidFnPtr *callbacks = getCallbacks(env, cls);
	callbacks[MaxObject_doError](env, cls, bytes);
}


/*
 * Call ouchstring on the message.  (Static method.)
 */
JNIEXPORT void JNICALL Java_com_cycling74_max_MaxObject_doOuch(
		JNIEnv *env,
		jclass cls,
		jbyteArray bytes) {
		
	VoidFnPtr *callbacks = getCallbacks(env, cls);
	callbacks[MaxObject_doOuch](env, cls, bytes);
}

/*
 * Call post on the message.  (Static method.)
 */
JNIEXPORT void JNICALL Java_com_cycling74_max_MaxObject_doPost(
		JNIEnv *env,
		jclass cls,
		jbyteArray bytes) {
		
	VoidFnPtr *callbacks = getCallbacks(env, cls);
	callbacks[MaxObject_doPost](env, cls, bytes);
}


/*
 * Return the Max version number.  (Static method.)
 */
JNIEXPORT jshort JNICALL Java_com_cycling74_max_MaxObject_doGetMaxVersion(
		JNIEnv *env,
		jclass cls) {
	VoidFnPtr *callbacks = getCallbacks(env, cls);
	return (jshort)callbacks[MaxObject_doGetMaxVersion](env, cls);
}

//
// Outlet methods
//

/*
 * Java wants to send an int out an outlet.  (Instance method.)
 */ 
JNIEXPORT jboolean JNICALL Java_com_cycling74_max_MaxObject_doOutlet__II(
		JNIEnv *env, 
		jobject self,
		jint index, 
		jint arg) {
	
	jclass cls = (*env)->GetObjectClass(env, self);
	VoidFnPtr *callbacks = getCallbacks(env, cls);
	return (jboolean)callbacks[MaxObject_doOutlet__II](env, self, index, arg);
}

/*
 * Java wants to send a float out an outlet.  (Instance method.)
 */ 
JNIEXPORT jboolean JNICALL Java_com_cycling74_max_MaxObject_doOutlet__IF(
		JNIEnv *env,
		jobject self,
		jint index,
		jfloat arg) {
		
	jclass cls = (*env)->GetObjectClass(env, self);
	VoidFnPtr *callbacks = getCallbacks(env, cls);
	return (jboolean)callbacks[MaxObject_doOutlet__IF](env, self, index, arg);
}


/*
 * Java wants to send a list out an outlet.  We open-code the index of the outlet
 * in the defer function itself.
 * (Instance method.)
 */ 
JNIEXPORT jboolean JNICALL Java_com_cycling74_max_MaxObject_doOutlet__I_3Lcom_cycling74_max_Atom_2(
		JNIEnv *env,
		jobject self,
		jint index,
		jobjectArray arg) {
		
	jclass cls = (*env)->GetObjectClass(env, self);
	VoidFnPtr *callbacks = getCallbacks(env, cls);
	return (jboolean)callbacks[MaxObject_doOutlet__I_3Lcom_cycling74_max_Atom_2](env, self, index, arg);
}

/*
 * Java wants to send a message (String+args) out an outlet.  We open-code the index of the outlet
 * in the defer function itself.
 * (Instance method.)
 */ 
JNIEXPORT jboolean JNICALL Java_com_cycling74_max_MaxObject_doOutlet__ILjava_lang_String_2_3Lcom_cycling74_max_Atom_2(
		JNIEnv *env,
		jobject self,
		jint index,
		jstring msg,
		jobjectArray arg) {

	jclass cls = (*env)->GetObjectClass(env, self);
	VoidFnPtr *callbacks = getCallbacks(env, cls);
	return (jboolean)callbacks[MaxObject_doOutlet__ILjava_lang_String_2_3Lcom_cycling74_max_Atom_2](
				env, self, index, msg, arg);
}

/*
 * It's time to take down the splash applet.
 */
JNIEXPORT void JNICALL Java_com_cycling74_max_MaxObject_doCloseSplash(
		JNIEnv *env,
		jclass cls) {
		
	VoidFnPtr *callbacks = getCallbacks(env, cls);
	callbacks[MaxObject_doCloseSplash](env, cls);
}
		

 //
//// Utilities
 //

/*
 * Get the Java class's callbacks field, which points to the combined
 * instance/class callbacks table.
 */
VoidFnPtr *getCallbacks(JNIEnv *env, jclass maxObjectClass) {
	static jclass frameworkClass = NULL;
	static jfieldID callbacksTable_FID = NULL;
	static jlong callbacks = NULL;
	
	if (frameworkClass == NULL) {
		frameworkClass = getClassByName(env, MAX_FRAMEWORK_CLASS);
	}
	if (callbacksTable_FID == NULL) {
		callbacksTable_FID = (*env)->GetStaticFieldID(env, frameworkClass,
												      CALLBACKS_TABLE_FNAME,
													  CALLBACKS_TABLE_SIG);
    }
	if (callbacks == NULL) {
		callbacks = (*env)->GetStaticLongField(env, maxObjectClass,
											   callbacksTable_FID);
    }
	return (VoidFnPtr *)callbacks;
}

/*
 * Get a class by name.
 */
jclass getClassByName(JNIEnv *env, char *className) {

	jclass l_class = (*env)->FindClass(env, className);
	jclass g_class;
	if (l_class == NULL) {
		return NULL;
	}
	g_class = (*env)->NewGlobalRef(env, l_class);
	(*env)->DeleteLocalRef(env, l_class);
    if (g_class == NULL) {
    	return NULL;
    }
    return g_class;
}
