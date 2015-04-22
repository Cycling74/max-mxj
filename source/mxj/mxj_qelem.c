#include "maxjava.h"
#include "threadenv.h"
#include "classes.h"
#include "ExceptionUtils.h"
#include "mxj_qelem.h"

#define MXJ_QELEM_QFN_METHOD    "qfn"
#define MXJ_QELEM_CLASSNAME "com/cycling74/max/MaxQelem"
#define MXJ_QELEM_QPTR_FIELD "_p_qelem"


extern JavaVM *g_jvm;

static jmethodID s_qelem_qfn_mid;
static jclass    s_qelem_clazz;
static jfieldID  s_qelem_ptr_fid;

typedef struct mxj_qelem
{
	void *qelem;
	jobject obj;
	JNIEnv *env;
	
} mxj_qelem;

void JNICALL mxj_qelem_free(JNIEnv *env, jobject obj);
jlong JNICALL mxj_qelem_new(JNIEnv *env, jobject obj);
void JNICALL mxj_qelem_set(JNIEnv *env, jobject obj);
void JNICALL mxj_qelem_front(JNIEnv *env, jobject obj);
void JNICALL mxj_qelem_unset(JNIEnv *env, jobject obj);

mxj_qelem *get_qelem_pointer(JNIEnv *env, jobject obj);
void mxj_qelem_qfn(mxj_qelem*);
void mxj_qelem_register_natives(JNIEnv*,jclass );

mxj_qelem *get_qelem_pointer(JNIEnv *env, jobject obj)
{
	return (mxj_qelem *)(t_atom_long)(MXJ_JNI_CALL(env,GetLongField)(env, obj,s_qelem_ptr_fid));
}

/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    _free_clock
 * Signature: ()V
 * called by the release method, or, if that step was skipped,
 * by the finalize method when the java instance is GC'd
 */
void JNICALL mxj_qelem_free(JNIEnv *env, jobject obj)
{
	mxj_qelem *x = get_qelem_pointer(env, obj);
	if (x) {
		MXJ_JNI_CALL(env,DeleteWeakGlobalRef)(env, x->obj);
		checkException(env);
		if (x->qelem)
			qelem_free(x->qelem);
		sysmem_freeptr(x);
	}
}

/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    _create_max_clock
 * Signature: ()J
 */
jlong JNICALL mxj_qelem_new(JNIEnv *env, jobject obj)
{
	mxj_qelem *x;
	
	x = (mxj_qelem*)sysmem_newptr(sizeof(mxj_qelem));
	x->qelem =  qelem_new(x,(method)mxj_qelem_qfn);
	x->obj = MXJ_JNI_CALL(env,NewWeakGlobalRef)(env,obj);
	x->env = env;
	
	return (jlong) (t_atom_long)x;
}

/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    clockDelay
 * Signature: (D)V
 */
void JNICALL mxj_qelem_set(JNIEnv *env, jobject obj)
{
  	mxj_qelem *x = get_qelem_pointer(env, obj);
  	qelem_set(x->qelem);
}

/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    clockDelay
 * Signature: (D)V
 */
void JNICALL mxj_qelem_front(JNIEnv *env, jobject obj)
{
  	mxj_qelem *x = get_qelem_pointer(env, obj);
  	qelem_front(x->qelem);
}

/*
 * Class:     com_cycling74_max_MaxClock
 * Method:    clockUnset
 * Signature: ()V
 */
void JNICALL mxj_qelem_unset(JNIEnv *env, jobject obj)
{
	mxj_qelem *x = get_qelem_pointer(env, obj);
	qelem_unset(x->qelem);
}

void init_mxj_qelem(JNIEnv *env)
{
	s_qelem_clazz         =  getClassByName(env, MXJ_QELEM_CLASSNAME);
	s_qelem_qfn_mid		=  MXJ_JNI_CALL(env,GetMethodID)(env,s_qelem_clazz,MXJ_QELEM_QFN_METHOD,"()V");
	checkException(env);
	s_qelem_ptr_fid =  MXJ_JNI_CALL(env,GetFieldID)(env, s_qelem_clazz, MXJ_QELEM_QPTR_FIELD, "J");
    checkException(env);
    mxj_qelem_register_natives(env,s_qelem_clazz);
    checkException(env);
}

void mxj_qelem_qfn(mxj_qelem *x)
{
	THREADENV(x->env);
	MXJ_JNI_CALL(x->env,CallVoidMethod)(x->env, x->obj, s_qelem_qfn_mid);
    checkException(x->env);
}

void mxj_qelem_register_natives(JNIEnv *env,jclass clazz)
{
	int count=0;
	JNINativeMethod methods[] =
	{
   		{ "_mxj_qelem_new", "()J", mxj_qelem_new },
   		{ "_mxj_qelem_free","()V", mxj_qelem_free  },
   		{ "set","()V", mxj_qelem_set },
  		{ "front","()V", mxj_qelem_front },
   		{ "unset", "()V", mxj_qelem_unset },
   		{ NULL, NULL, NULL }
	};
	
	while (methods[count].name) count++;
	
	//When adding methods dont forget to update the number of methods being passed to RegisterNatives
	MXJ_JNI_CALL(env,RegisterNatives)(env,clazz,methods,count);
	checkException(env);
}
