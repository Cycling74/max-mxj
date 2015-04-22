#ifdef MAC_VERSION
#include <JavaVM/jni.h>        // Java Native Interface definitions
#include <JavaVM/jni_md.h>
#else
#include "jni.h"
#include "jni_md.h"
#endif
#include "maxjava.h"
#include "threadenv.h"
#include "classes.h"
#include "clock_callbacks.h"
#include "ExceptionUtils.h"

#define MXJ_MAX_CLOCK_TICK_METHOD    "tick"
#define MXJ_MAX_CLOCK_CLASSNAME "com/cycling74/max/MaxClock"
#define MXJ_MAX_CLOCK_CPTR_FIELD "_p_clock"


extern JavaVM *g_jvm;

jmethodID g_mxj_tick_method_id;
jclass    g_max_clock_clazz;
jfieldID  g_mxj_myclockptr_field_id;


typedef struct myClock {
	void *cl;
	jobject obj;
	JNIEnv *env;
	
} myClock;

void JNICALL mxj_free_clock(JNIEnv *, jobject);
jlong JNICALL mxj_create_max_clock(JNIEnv *env, jobject obj);
void JNICALL mxj_clock_delay(JNIEnv *env, jobject obj, jdouble interval);
void JNICALL mxj_clock_unset(JNIEnv *env, jobject obj);
jdouble JNICALL mxj_gettime(JNIEnv *env, jobject obj);

myClock *get_my_clock_pointer(JNIEnv *env, jobject obj);
void init_clock_callbacks(JNIEnv*);
void max_clock_tick(myClock*);
void mxj_max_clock_register_natives(JNIEnv*,jclass );


myClock *get_my_clock_pointer(JNIEnv *env, jobject obj)
{
	return (myClock *)(t_atom_long)(MXJ_JNI_CALL(env,GetLongField)(env, obj,g_mxj_myclockptr_field_id));
}

/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    _free_clock
 * Signature: ()V
 * called by the finalize method...when the java instance is GC'd
 */
void JNICALL mxj_free_clock(JNIEnv *env, jobject obj)
{
	myClock *x = get_my_clock_pointer(env, obj);
	if (x) {
		MXJ_JNI_CALL(env,DeleteWeakGlobalRef)(env, x->obj);
		checkException(env);
		if (x->cl) {
			clock_unset(x->cl);
			freeobject(x->cl);
		}
		sysmem_freeptr(x);
	}
}

/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    _create_max_clock
 * Signature: ()J
 */
jlong JNICALL mxj_create_max_clock(JNIEnv *env, jobject obj)
{
	myClock *x;
	
	x = (myClock*)sysmem_newptr(sizeof(myClock));
	x->cl =  clock_new(x,(method)max_clock_tick);
	x->obj = MXJ_JNI_CALL(env,NewWeakGlobalRef)(env,obj);
	x->env = env;
	
	return p_to_jlong(x);
}



/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    clockDelay
 * Signature: (D)V
 */
void JNICALL mxj_clock_delay(JNIEnv *env, jobject obj, jdouble interval)
{
  	myClock *x = get_my_clock_pointer(env, obj);
  	clock_fdelay(x->cl,(double)interval);
	
}

/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    clockUnset
 * Signature: ()V
 */
void JNICALL mxj_clock_unset(JNIEnv *env, jobject obj)
{
	myClock *x = get_my_clock_pointer(env, obj);
	if (x && x->cl)
		clock_unset(x->cl);
}

/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    getTime
 * Signature: ()D
 */
jdouble JNICALL mxj_gettime(JNIEnv *env, jobject obj)
{
 	double d;
    clock_getftime(&d);
 	return (jdouble) d;
}

void init_clock_callbacks(JNIEnv *env)
{
	g_max_clock_clazz         =  getClassByName(env, MXJ_MAX_CLOCK_CLASSNAME);
	g_mxj_tick_method_id      =  MXJ_JNI_CALL(env,GetMethodID)(env,g_max_clock_clazz,MXJ_MAX_CLOCK_TICK_METHOD,"()V");
	checkException(env);
	g_mxj_myclockptr_field_id =  MXJ_JNI_CALL(env,GetFieldID)(env, g_max_clock_clazz, MXJ_MAX_CLOCK_CPTR_FIELD, "J");
    checkException(env);
    mxj_max_clock_register_natives(env,g_max_clock_clazz);
    checkException(env);
}

void max_clock_tick(myClock *x)
{
	THREADENV(x->env);
	MXJ_JNI_CALL(x->env,CallVoidMethod)(x->env, x->obj, g_mxj_tick_method_id);
    checkException(x->env);
}

void mxj_max_clock_register_natives(JNIEnv *env,jclass clazz)
{
	int count=0;
	JNINativeMethod methods[] =
	{
   		{ "_create_max_clock", "()J", mxj_create_max_clock },
   		{ "_free_clock","()V", mxj_free_clock  },
   		{ "delay","(D)V", mxj_clock_delay },
  		{ "unset","()V", mxj_clock_unset },
   		{ "getTime", "()D", mxj_gettime },
   		{ NULL, NULL, NULL }
	};
	
	while (methods[count].name) count++;
	
	//When adding methods dont forget to update the number of methods being passed to RegisterNatives
	MXJ_JNI_CALL(env,RegisterNatives)(env,clazz,methods,count);
	checkException(env);
}
