/*
 * Callbacks from Java into Max.
 * These functions are called from the native methods in maxjavactx.c.
 */

// Java Native Interface
#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif
#include "classes.h"
#include "maxjava.h"
#include "ExceptionUtils.h"
#include "callbacks.h"
#include "Atom.h"
#include "mxj_methodlist.h"
#include "mxj_attr.h"
#include "mxj_utils.h"
#include "maxjava.h"
#include "ext_atomic.h"

#define MXJ_MAX_OBJECT_CLASSNAME "com/cycling74/max/MaxObject"

//globals
jclass g_max_object_clazz;
extern JavaVM *g_jvm;

extern void* g_sched_peer;//points to our peer while java obj is being constructed
extern void* g_main_peer;//ditto for main thread

void JNICALL mxj_error(JNIEnv*,jclass,jbyteArray);
void JNICALL mxj_post(JNIEnv*,jclass,jbyteArray);
void JNICALL mxj_ouch(JNIEnv*,jclass,jbyteArray);
jboolean JNICALL mxj_outlet_bang(JNIEnv *, jobject , jint );
jboolean JNICALL mxj_outlet_int(JNIEnv *, jobject,jint , jint );
jboolean JNICALL mxj_outlet_float(JNIEnv*,jobject,jint,jfloat);
jboolean JNICALL mxj_outlet_list(JNIEnv*,jobject ,jint ,jobjectArray);
jboolean JNICALL mxj_outlet_list_int(JNIEnv*,jobject ,jint ,jintArray);
jboolean JNICALL mxj_outlet_list_int_msg(JNIEnv*,jobject ,jint ,jstring,jintArray);
jboolean JNICALL mxj_outlet_list_int_msg_high(JNIEnv*,jobject ,jint ,jstring,jintArray);
jboolean JNICALL mxj_outlet_list_float(JNIEnv*,jobject ,jint ,jfloatArray);
jboolean JNICALL mxj_outlet_list_float_msg(JNIEnv*,jobject ,jint ,jstring,jfloatArray);
jboolean JNICALL mxj_outlet_list_float_msg_high(JNIEnv*,jobject ,jint ,jstring,jfloatArray);
jboolean JNICALL mxj_outlet_anything(JNIEnv*,jobject ,jint ,jstring,jobjectArray);
jboolean JNICALL mxj_outlet_bang_high(JNIEnv *, jobject , jint );
jboolean JNICALL mxj_outlet_int_high(JNIEnv *, jobject,jint , jint );
jboolean JNICALL mxj_outlet_float_high(JNIEnv*,jobject,jint,jfloat);
jboolean JNICALL mxj_outlet_list_high(JNIEnv*,jobject ,jint ,jobjectArray);
jboolean JNICALL mxj_outlet_list_int_high(JNIEnv*,jobject ,jint ,jintArray);
jboolean JNICALL mxj_outlet_list_float_high(JNIEnv*,jobject ,jint ,jfloatArray);
jboolean JNICALL mxj_outlet_anything_high(JNIEnv*,jobject ,jint ,jstring,jobjectArray);

jint JNICALL mxj_getinlet(JNIEnv* , jobject );
jlong JNICALL mxj_get_parent_patcher(JNIEnv* , jobject );
jlong JNICALL mxj_get_max_box(JNIEnv* env, jobject obj );

void JNICALL mxj_close_splash(JNIEnv*,jclass );

void mxj_max_object_register_natives(JNIEnv*,jclass);
void JNICALL mxj_register_message(JNIEnv *, jobject , jstring,jint, jstring , jstring, jstring);
void JNICALL mxj_register_attribute(JNIEnv *env, jobject obj, jobject AttrInfo);
void JNICALL mxj_embed_message(JNIEnv *env,jobject obj, jstring msg, jobjectArray arg);

/* tells us whether we are a Max thread or Java thread */
short i_am_a_max_thread()
{
	return (systhread_istimerthread() || systhread_ismainthread());
}

short i_am_in_java_constructor()
{
    return(g_sched_peer || g_main_peer);
}


// Tell the user something (via deferral)
void say_deferred(t_maxjava *x,
				  t_symbol *s,
				  short argc,
				  t_atom argv[]);

// Get the Max mxj object that's the Java object's peer
t_maxjava *getPeer(JNIEnv *env, jobject maxObjectObj);
//...this version can be called from the constructor
t_maxjava *getPeerValidInConst(JNIEnv *env, jobject maxObjectObj);

// Get the length of a string
#ifdef WIN_VERSION
size_t strlen(const char *s);
#endif

/*
 * The reason we need to have the wrapper for float has to do with the way
 * floating point arguments are passed on the stack--or rather how for
 * efficiency's sake, the stack is actually passed in registers.
 */
void do_outlet_int_atom(void *x, t_symbol *s, short argc, t_atom argv[]);
void do_outlet_float_atom(void *x, t_symbol *s, short argc, t_atom argv[]);

//
//// Formatted text output to the user.
//

/*
 * Call error/ouchstring/post
 */
void say(t_outputfunc outputFn, JNIEnv *env, jclass cls, jbyteArray bytes)
{
	jint length = MXJ_JNI_CALL(env,GetArrayLength)(env, bytes);
	t_atom deferArgs[2];
	char *message;
	
	message = mxj_getbytes(length+1);
	if (message == 0) {
		JNU_ThrowByName(env, "java/lang/OutOfMemoryError", 0);
		return;
	}
	
	MXJ_JNI_CALL(env,GetByteArrayRegion)(env, bytes, 0, length, (jbyte *)message);
	message[length] = 0;
	
	if(systhread_ismainthread()) {	// if we are executing in lowpriority do action right away
		outputFn(NULL, "%s", message);
		mxj_freebytes(message, length+1);
	}
	else {	// call defer low on it. message is freed in say_deferred
		A_SETLONG(&deferArgs[0], (t_atom_long)outputFn);
		A_SETLONG(&deferArgs[1], (t_atom_long)message);
		defer_low(NULL, (method)say_deferred, 0, 2, deferArgs);
	}	
}


/*
 * Deferred output function - used to call post, error, and ouchstring
*/
void say_deferred(t_maxjava *x,   t_symbol *s,  short argc,  t_atom argv[])
{
	t_outputfunc outputFn = (void *)argv[0].a_w.w_long;
	char *message = (char *)argv[1].a_w.w_long;
	long length;

	if (message) {
		length = (long)strlen(message);
		outputFn(NULL, "%s", message);
		mxj_freebytes(message, length+1);
	}
}

/*
 * Deferred outlet functions
 */
void do_outlet_int_atom(void *x, t_symbol *s, short argc, t_atom argv[])
{
	outlet_int(x, atom_getlong(&argv[0]));
}

/*
 * Process a deferred call to outlet_float.
 */
void do_outlet_float_atom(void *x, t_symbol *s, short argc, t_atom argv[])
{
	outlet_float(x, atom_getfloat(&argv[0]));
}

//
//// Utility functions
//

/*
 * Get the Java instance's peer field, which points at the t_maxjava that created it.
 */
t_maxjava *getPeer(JNIEnv *env, jobject maxObjectObj)
{
	jclass maxObjectClass = MXJ_JNI_CALL(env,GetObjectClass)(env, maxObjectObj);
	jfieldID peer_FID;
	jlong peer;
	
	//do we need to always get this? it is probably always coming from maxObject
	peer_FID = MXJ_JNI_CALL(env,GetFieldID)(env, maxObjectClass, PEER_FNAME, PEER_SIG);
	peer = MXJ_JNI_CALL(env,GetLongField)(env, maxObjectObj, peer_FID);
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,maxObjectClass);
	
	return (t_maxjava *)(t_atom_long)peer;
}

t_maxjava *getPeerValidInConst(JNIEnv *env, jobject maxObjectObj) {
	jclass maxObjectClass = MXJ_JNI_CALL(env,GetObjectClass)(env, maxObjectObj);
	jfieldID peer_FID;
	jlong peer;
	
	if(i_am_in_java_constructor())
	{
		if(systhread_ismainthread())
			return (t_maxjava*)g_main_peer;
		else if(systhread_istimerthread())
			return (t_maxjava*)g_sched_peer;
		else
		{   //shouldn't happen
			error("(mxj) unable to get peer in constructor.");
			return(0);
		}
	}
	else
	{
		peer_FID = MXJ_JNI_CALL(env,GetFieldID)(env, maxObjectClass, PEER_FNAME, PEER_SIG);
		peer = MXJ_JNI_CALL(env,GetLongField)(env, maxObjectObj, peer_FID);
	}
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,maxObjectClass);
	return (t_maxjava *)(t_atom_long)peer;
}

//// Java callbacks.

/*
 * Call error on the message using defer_low.
 */
void JNICALL mxj_error(JNIEnv *env,  jclass cls, jbyteArray bytes)
{
	say(object_error, env, cls, bytes);
}

/*
 * Call ouchstring on the message using defer_low.
 */
void JNICALL mxj_ouch(JNIEnv *env, jclass cls, jbyteArray bytes)
{	
	say(object_error_obtrusive, env, cls, bytes);
}

/*
 * Call post on the message using defer_low.
 */
void JNICALL mxj_post(JNIEnv *env, jclass cls,  jbyteArray bytes)
{
	say(object_post, env, cls, bytes);
}

/*
 * Save message in patcher
 */
void JNICALL mxj_embed_message(JNIEnv *env,jobject obj, jstring msg, jobjectArray arg)
{
	t_maxjava *maxObj = getPeer(env, obj);
	short argc, argc2;
	int i;
	t_symbol *s;
	t_atom *argv = NULL;
	t_atom *argv2 = NULL;
	
	if (maxObj == NULL || maxObj->binbuf == NULL) {
		object_error((t_object *)maxObj, "(mxj)embedMessage can only be called within the context of the save method.");
		return;
	}
	
	argc = 0;
	s = newSym(env, msg);
	argv = newArgv(env, arg, &argc);
	argc2 = argc + 2;
	
	if (argc > 4094) {	// inform that there's a truncation
		object_error((t_object *)maxObj, "(mxj)embedMessage(\"%s\",...) truncated to 4094 items", s->s_name);
		argc2 = 4094;
	}
	
	argv2 = (t_atom *)sysmem_newptr(argc2 * sizeof(t_atom));
	if (!argv2) {
		mxj_freebytes(argv,argc*sizeof(t_atom));
		object_error((t_object *)maxObj, "(mxj)could not allocate enough memory for embedMessage(\"%s\", ...)", s->s_name);
		return;
	}
	
	A_SETSYM(argv2,gensym("#X"));
	A_SETSYM(argv2+1,s);
	
	for (i=0;i<MIN(argc, argc2-2);i++)
		argv2[i+2] = argv[i];
	
	binbuf_insert(maxObj->binbuf,gensym("#X"),argc2,argv2);
	mxj_freebytes(argv,argc*sizeof(t_atom));
	sysmem_freeptr(argv2);	// free the memory
	argv2 = NULL;
}


//
// Outlet methods
//

/*
 * Java wants to send a bang out an outlet.
 */
jboolean JNICALL mxj_outlet_bang(JNIEnv *env, jobject self,  jint index)
{
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		object_error((t_object *)maxObj, "bad outlet index.");
		return false;
	}
	
  	if(i_am_a_max_thread()) // outlet now
  	  	return (outlet_bang(maxObj->m_outlet[index]) != NULL);
  	else
		return(defer_low(maxObj->m_outlet[index], (method)outlet_bang, (t_symbol *)NULL, 0, NULL) != NULL);
}


/*
 * Java wants to send an int out an outlet.
 */
jboolean JNICALL mxj_outlet_int(JNIEnv *env, jobject self, jint index, jint arg)
{
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
  	if(i_am_a_max_thread()) // outlet now
  	  	return (outlet_int(maxObj->m_outlet[index], arg) != NULL);
  	else {
		t_atom deferArg;
		atom_setlong(&deferArg, arg);
		return (defer_low(maxObj->m_outlet[index], (method)do_outlet_int_atom,  NULL, 1, &deferArg) != NULL);
	}
}

/*
 * Java wants to send a float out an outlet.
 */
jboolean JNICALL mxj_outlet_float(JNIEnv *env, jobject self, jint index, jfloat arg)
{
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
    if(i_am_a_max_thread()) //outlet immediately
        return (outlet_float(maxObj->m_outlet[index], arg) != NULL);
    else{
		t_atom deferArg;
		atom_setfloat(&deferArg, arg);
		return(defer_low(maxObj->m_outlet[index], (method)do_outlet_float_atom, NULL, 1, &deferArg) != NULL);
	}
}

/*
 * Java wants to send a list out an outlet.  We open-code the index of the outlet
 * in the defer function itself.
 */
jboolean JNICALL mxj_outlet_list(JNIEnv *env, jobject self, jint index, jobjectArray arg)
{
	short argc;
	t_atom *argv;
	jboolean rv;
	
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgv(env, arg, &argc);
		
	if (i_am_a_max_thread())
		rv =  (outlet_list(maxObj->m_outlet[index], 0L, argc, argv) != NULL);
	else // i am a java thread
		rv = (defer_low(maxObj->m_outlet[index],(method)outlet_list, 0L, argc, argv) != NULL);
	mxj_freebytes(argv,argc*sizeof(t_atom));
	return rv;
}

/*
 * Optimized list output for when java passes in the list as an int array.
 */
jboolean JNICALL mxj_outlet_list_int(JNIEnv *env, jobject self, jint index, jintArray arg)
{
	short argc;
	t_atom *argv;
	jboolean rv;
	
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgvFromIntArray(env, arg, &argc);
	
	if (i_am_a_max_thread())
		rv =  (outlet_list(maxObj->m_outlet[index], 0L, argc, argv) != NULL);
	else // i am a java thread
		rv = (defer_low(maxObj->m_outlet[index],(method)outlet_list, 0L, argc, argv) != NULL);
	mxj_freebytes(argv,argc*sizeof(t_atom));

	return rv;
}

/*
 * Optimized list output for when java passes in the list as a float array.
 */
jboolean JNICALL mxj_outlet_list_float(JNIEnv *env, jobject self, jint index, jfloatArray arg)
{	
	short argc;
	t_atom *argv;
	jboolean rv;
	
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgvFromFloatArray(env, arg, &argc);
	
	if (i_am_a_max_thread())
		rv =  (outlet_list(maxObj->m_outlet[index], 0L, argc, argv) != NULL);
	else // i am a java thread
		rv = (defer_low(maxObj->m_outlet[index],(method)outlet_list, 0L, argc, argv) != NULL);
	mxj_freebytes(argv,argc*sizeof(t_atom));
	
	return rv;
}

/*
 * Optimized list output for when java passes in the list as an int array with a message argument
 */
jboolean JNICALL mxj_outlet_list_int_msg(JNIEnv *env, jobject self,  jint index, jstring msg,  jintArray arg)
{
	
	short argc;
	t_atom *argv;
	jboolean rv;
	t_symbol *msgSym;
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgvFromIntArray(env, arg, &argc);
	msgSym = newSym(env, msg);
	
	if (i_am_a_max_thread()) {
		rv =  (outlet_anything(maxObj->m_outlet[index], msgSym, argc, argv) != NULL);
	}
	else {	// i am a java thread
		rv = (defer_low(maxObj->m_outlet[index],(method)outlet_anything, msgSym, argc, argv) != NULL);
	}
	
	mxj_freebytes(argv,argc*sizeof(t_atom));
	
	return rv;
}

/*
 * Optimized list output for when java passes in the list as a float array with a message argument
 */
jboolean JNICALL mxj_outlet_list_float_msg(JNIEnv *env, jobject self, jint index, jstring msg, jfloatArray arg)
{
	short argc;
	t_atom *argv;
	jboolean rv;
	t_symbol *msgSym;
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgvFromFloatArray(env, arg, &argc);
	msgSym = newSym(env, msg);
	
	if (i_am_a_max_thread()) {
		rv =  (outlet_anything(maxObj->m_outlet[index], msgSym, argc, argv) != NULL);
	}
	else { // i am a java thread
		rv = (defer_low(maxObj->m_outlet[index],(method)outlet_anything, msgSym, argc, argv) != NULL);
	}
	mxj_freebytes(argv,argc*sizeof(t_atom));
	
	return rv;
}

/*
 * Java wants to send a message (String+args) out an outlet.  We open-code the index of the outlet
 * in the defer function itself.
 */
jboolean JNICALL mxj_outlet_anything(JNIEnv *env, jobject self, jint index, jstring msg, jobjectArray arg)
{	
	short argc;
	t_symbol *msgSym;
	t_atom *argv;
	jboolean rv;
	
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	msgSym = newSym(env, msg);
	argv = newArgv(env, arg, &argc);
	
	if (i_am_a_max_thread()) {
		rv =  (outlet_anything(maxObj->m_outlet[index], msgSym, argc, argv) != NULL);
	}
	else {	// i am a java thread
		rv = (defer_low(maxObj->m_outlet[index],(method)outlet_anything, msgSym, argc, argv) != NULL);
	}
	
	mxj_freebytes(argv,argc*sizeof(t_atom));
	return rv;
}

/*
 * Java wants to send a bang out an outlet.
 */
jboolean JNICALL mxj_outlet_bang_high(JNIEnv *env, jobject self, jint index)
{
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
  	if(systhread_istimerthread())	// outlet now
  	  	return (outlet_bang(maxObj->m_outlet[index]) != NULL);
  	else {
		schedule_delay(maxObj->m_outlet[index], (method)outlet_bang, 0, (t_symbol *)NULL, 0, NULL);
		return TRUE;
	}
}

/*
 * Java wants to send an int out an outlet.
 */
jboolean JNICALL mxj_outlet_int_high(JNIEnv *env, jobject self, jint index, jint arg)
{
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
  	if(systhread_istimerthread()) // outlet now
  	  	return (outlet_int(maxObj->m_outlet[index], arg) != NULL);
  	else {
		schedule_delay(maxObj->m_outlet[index], (method)outlet_int, 0, (t_symbol *)(t_atom_long)arg, 0, NULL);
		return TRUE;
	}
}

/*
 * Java wants to send a float out an outlet.
 */
jboolean JNICALL mxj_outlet_float_high(JNIEnv *env, jobject self, jint index, jfloat arg)
{	
	t_atom deferArgs[2];
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
    if (systhread_istimerthread())	// outlet immediately
        return (outlet_float(maxObj->m_outlet[index], arg) != NULL);
    else {
		A_SETFLOAT(&deferArgs[0], arg);
		schedule_delay(maxObj->m_outlet[index], (method)do_outlet_float_atom, 0, NULL, 1, deferArgs);
		return TRUE;
	}
}

/*
 * Java wants to send a list out an outlet.  We open-code the index of the outlet
 * in the defer function itself.
 */
jboolean JNICALL mxj_outlet_list_high(JNIEnv *env, jobject self, jint index, jobjectArray arg)
{
	short argc;
	t_atom *argv;
	jboolean rv;
	
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgv(env, arg, &argc);
	
	if (systhread_istimerthread())
		rv =  (outlet_list(maxObj->m_outlet[index], 0L, argc, argv) != NULL);
	else {
		schedule_delay(maxObj->m_outlet[index],(method)outlet_list, 0, 0L, argc, argv);
		rv = TRUE;
	}
	
	mxj_freebytes(argv,argc*sizeof(t_atom));
	return rv;
}

/*
 * Optimized list output for when java passes in the list as an int array.
 */
jboolean JNICALL mxj_outlet_list_int_high(JNIEnv *env, jobject self, jint index, jintArray arg)
{
	short argc;
	t_atom *argv;
	jboolean rv;
	
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgvFromIntArray(env, arg, &argc);
	
	if (systhread_istimerthread())
		rv =  (outlet_list(maxObj->m_outlet[index], 0L, argc, argv) != NULL);
	else {
		schedule_delay(maxObj->m_outlet[index],(method)outlet_list, 0, 0L, argc, argv);
		rv = TRUE;
	}
	
	mxj_freebytes(argv,argc*sizeof(t_atom));
	return rv;
}

/*
 * Optimized list output for when java passes in the list as a float array.
 */
jboolean JNICALL mxj_outlet_list_float_high(JNIEnv *env, jobject self, jint index, jfloatArray arg)
{	
	short argc;
	t_atom *argv;
	jboolean rv;
	
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgvFromFloatArray(env, arg, &argc);
	
	if (systhread_istimerthread())
		rv =  (outlet_list(maxObj->m_outlet[index], 0L, argc, argv) != NULL);
	else {
		schedule_delay(maxObj->m_outlet[index],(method)outlet_list, 0, 0L, argc, argv);
		rv = TRUE;
	}
	
	mxj_freebytes(argv,argc*sizeof(t_atom));
	return rv;
}

/*
 * Optimized list output for when java passes in the list as an int array.
 */
jboolean JNICALL mxj_outlet_list_int_msg_high(JNIEnv *env, jobject self, jint index, jstring msg, jintArray arg)
{
	short argc;
	t_atom *argv;
	jboolean rv;
	t_symbol *msgSym;
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgvFromIntArray(env, arg, &argc);
	msgSym = newSym(env, msg);
		
	if (i_am_a_max_thread()) {
		rv =  (outlet_anything(maxObj->m_outlet[index], msgSym, argc, argv) != NULL);
	}
	else {	// i am a java thread
		rv = (defer_low(maxObj->m_outlet[index],(method)outlet_anything, msgSym, argc, argv) != NULL);
	}
	
	mxj_freebytes(argv,argc*sizeof(t_atom));
	return rv;
}

/*
 * Optimized list output for when java passes in the list as a float array.
 */
jboolean JNICALL mxj_outlet_list_float_msg_high(JNIEnv *env, jobject self, jint index, jstring msg, jfloatArray arg)
{
	short argc;
	t_atom *argv;
	jboolean rv;
	t_symbol *msgSym;
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	argv = newArgvFromFloatArray(env, arg, &argc);
	msgSym = newSym(env, msg);
	
	if (i_am_a_max_thread()) {
		rv =  (outlet_anything(maxObj->m_outlet[index], msgSym, argc, argv) != NULL);
	}
	else {	// i am a java thread
		rv = (defer_low(maxObj->m_outlet[index],(method)outlet_anything, msgSym, argc, argv) != NULL);
	}

	mxj_freebytes(argv,argc*sizeof(t_atom));
	return rv;
}

/*
 * Java wants to send a message (String+args) out an outlet.  We open-code the index of the outlet
 * in the defer function itself.
 */
jboolean JNICALL mxj_outlet_anything_high(JNIEnv *env, jobject self, jint index, jstring msg, jobjectArray arg)
{	
	short argc;
	t_symbol *msgSym;
	t_atom *argv;
	jboolean rv;
	
	t_maxjava *maxObj = getPeer(env, self);
	
	if (maxObj == NULL) {
		return false;
	}
	
	if (index < 0 || index >= maxObj->m_nOutlets) {
		error("bad outlet index.");
		return false;
	}
	
	argc = 0;
	msgSym = newSym(env, msg);
	argv = newArgv(env, arg, &argc);
	
	if (systhread_istimerthread())
		rv =  (outlet_anything(maxObj->m_outlet[index], msgSym, argc, argv) != NULL);
	else {
		schedule_delay(maxObj->m_outlet[index],(method)outlet_anything, 0, msgSym, argc, argv);
		rv = TRUE;
	}
	
	mxj_freebytes(argv,argc*sizeof(t_atom));
	return rv;
}

/**
 * Close the splash screen applet.
 */
void JNICALL mxj_close_splash(JNIEnv *env, jclass cls)
{
	// not implemented
}

/*
 * Return the inlet of the last received message.
 */
jint JNICALL mxj_getinlet(JNIEnv *env, jobject obj)
{
	
 	t_maxjava *maxObj = getPeer(env, obj);
	
    if (maxObj == NULL) {
		return -1;
	}
	
	return (jint)proxy_getinlet((t_object *)maxObj);
}

/* returns a pointer to the thispatcher field of mxj object*/
jlong JNICALL mxj_get_parent_patcher(JNIEnv* env, jobject obj )
{
	t_maxjava *maxObj = getPeerValidInConst(env, obj);
    
	if (maxObj == NULL) {
		return 0;
	}
	
	return  p_to_jlong(maxObj->thispatcher);
}

/* returns a pointer to the thisbox field of the mxj object*/
/* since we have a save message defined the box isn't created */
/* until after the instance is and all incoming save messages are evaluated */
/* this means this is not valid in the constructor. boo hoo. */
jlong JNICALL mxj_get_max_box(JNIEnv* env, jobject obj)
{
	t_box *b;
	t_maxjava *maxObj = getPeerValidInConst(env, obj);
    b = 0;
	
    if (!maxObj || !maxObj->thispatcher)
		return 0;

	return p_to_jlong(maxObj->thisbox);
}

void JNICALL mxj_register_attribute(JNIEnv *env, jobject obj, jobject AttrInfo)
{
	t_mxj_attr *a;
	t_mxj_attr_desc ad;
	t_maxjava *maxObj;
	
	//from AttrInfo obj
	jstring ai_name;
	jstring ai_m_f_type;
	jstring ai_j_f_type;
	jstring ai_setter;
	jstring ai_setter_jptypes;
	jstring ai_setter_sig;
	jstring ai_getter;
	jstring ai_getter_jptypes;
	jstring ai_getter_sig;
	
	static jfieldID name_fid;
	static jfieldID m_f_type_fid;
	static jfieldID j_f_type_fid;
	static jfieldID setter_fid;
	static jfieldID setter_jptypes_fid;
	static jfieldID setter_sig_fid;
	static jfieldID getter_fid;
	static jfieldID getter_jptypes_fid;
	static jfieldID getter_sig_fid;
	static jfieldID settable_fid;
	static jfieldID gettable_fid;
	static jfieldID isvirtual_fid;
	
	
	static jclass   ai_clazz = NULL;
	
	static int      init = 0;
	MXJ_JNI_CALL(env,PushLocalFrame)(env,32);
	
	if(!init) {
		ai_clazz = MXJ_JNI_CALL(env, FindClass)(env,"com/cycling74/max/AttributeInfo");
		ai_clazz = MXJ_JNI_CALL(env,NewGlobalRef)(env,ai_clazz);
		name_fid       		= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"name","Ljava/lang/String;");
		m_f_type_fid       	= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"m_f_type","Ljava/lang/String;");
		j_f_type_fid       	= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"j_f_type","Ljava/lang/String;");
		setter_fid         	= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"setter","Ljava/lang/String;");
		setter_sig_fid     	= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"setter_sig","Ljava/lang/String;");
		setter_jptypes_fid 	= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"setter_jptypes","Ljava/lang/String;");
		getter_fid         	= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"getter","Ljava/lang/String;");
		getter_sig_fid     	= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"getter_sig","Ljava/lang/String;");
		getter_jptypes_fid 	= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"getter_jptypes","Ljava/lang/String;");
		settable_fid 		= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"settable","Z");
		gettable_fid 		= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"gettable","Z");
		isvirtual_fid 		= MXJ_JNI_CALL(env,GetFieldID)(env,ai_clazz,"isvirtual","I");
		init = 1;
	}
	
	ai_name          	= MXJ_JNI_CALL(env,GetObjectField)(env,AttrInfo,name_fid);
	ai_m_f_type      	= MXJ_JNI_CALL(env,GetObjectField)(env,AttrInfo,m_f_type_fid);
	ai_j_f_type      	= MXJ_JNI_CALL(env,GetObjectField)(env,AttrInfo,j_f_type_fid);
	ai_setter        	= MXJ_JNI_CALL(env,GetObjectField)(env,AttrInfo,setter_fid);
	ai_setter_jptypes 	= MXJ_JNI_CALL(env,GetObjectField)(env,AttrInfo,setter_jptypes_fid);
	ai_setter_sig    	= MXJ_JNI_CALL(env,GetObjectField)(env,AttrInfo,setter_sig_fid);
	ai_getter        	= MXJ_JNI_CALL(env,GetObjectField)(env,AttrInfo,getter_fid);
	ai_getter_jptypes 	= MXJ_JNI_CALL(env,GetObjectField)(env,AttrInfo,getter_jptypes_fid);
	ai_getter_sig    	= MXJ_JNI_CALL(env,GetObjectField)(env,AttrInfo,getter_sig_fid);
	
	ad.name     	  	= ai_name?(char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env, ai_name, NULL):NULL;
	ad.m_f_type       	= ai_m_f_type?(char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env, ai_m_f_type , NULL):NULL;
	ad.j_f_type       	= ai_j_f_type?(char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env, ai_j_f_type, NULL):NULL;
	ad.setter     	  	= ai_setter?(char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env, ai_setter, NULL):NULL;
	ad.setter_jptypes	= ai_setter_jptypes?(char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env, ai_setter_jptypes, NULL):NULL;
	ad.setter_sig     	= ai_setter_sig ?(char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env, ai_setter_sig, NULL):NULL;
	ad.getter     	  	= ai_getter?(char*)MXJ_JNI_CALL(env,GetStringUTFChars)(env, ai_getter, NULL):NULL;
	ad.getter_jptypes  	= ai_getter_jptypes?(char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env, ai_getter_jptypes, NULL):NULL;
	ad.getter_sig   	= ai_getter_sig?(char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env, ai_getter_sig, NULL):NULL;
	
	ad.settable  	  	= MXJ_JNI_CALL(env,GetBooleanField)(env,AttrInfo,settable_fid);
	ad.gettable  	  	= MXJ_JNI_CALL(env,GetBooleanField)(env,AttrInfo,gettable_fid);
	ad.isvirtual 	    = MXJ_JNI_CALL(env,GetIntField)(env,AttrInfo,isvirtual_fid);
	
 	maxObj = getPeer(env, obj);

    if (maxObj == NULL) {
    	if (ad.name) 			MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_name, ad.name);
		if (ad.m_f_type) 		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_m_f_type, ad.m_f_type);
		if (ad.j_f_type) 		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_j_f_type, ad.j_f_type);
		if (ad.setter) 			MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_setter, ad.setter);
		if (ad.setter_jptypes) 	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_setter_jptypes, ad.setter_jptypes);
		if (ad.setter_sig) 		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_setter_sig, ad.setter_sig);
		if (ad.getter) 		 	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_getter, ad.getter);
		if (ad.getter_jptypes) 	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_getter_jptypes, ad.getter_jptypes);
		if (ad.getter_sig) 		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_getter_sig, ad.getter_sig);
 		error("Big time barf: unable to get peer in mxj_register_attribute");
 		return;
 	}
	
	a   = (t_mxj_attr *)mxj_attr_new(env,maxObj->cls,&ad);
	mxj_add_attr(maxObj,a);
		
	if (ad.name) 			MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_name, ad.name);
	if (ad.m_f_type) 		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_m_f_type, ad.m_f_type);
	if (ad.j_f_type) 		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_j_f_type, ad.j_f_type);
	if (ad.setter) 			MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_setter, ad.setter);
	if (ad.setter_jptypes) 	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_setter_jptypes, ad.setter_jptypes);
	if (ad.setter_sig) 		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_setter_sig, ad.setter_sig);
	if (ad.getter) 		 	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_getter, ad.getter);
	if (ad.getter_jptypes) 	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_getter_jptypes, ad.getter_jptypes);
	if (ad.getter_sig) 		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, ai_getter_sig, ad.getter_sig);
	
	MXJ_JNI_CALL(env,PopLocalFrame)(env,NULL);
}

/*
 * Class:     com_cycling74_max_MaxObject
 * Method:    _register_message
 * Signature: (Ljava/lang/String;[I)V
 */
void JNICALL mxj_register_message(JNIEnv *env, jobject obj, jstring name,jint num_params, jstring java_sig,jstring mxj_paramtypes, jstring java_paramtypes)
{
	t_mxj_method *m;
	const char* m_pt;
	const char* j_pt;
	const char* m_name;
	const char* m_java_sig;
	
	jmethodID mid;
	
	t_maxjava *maxObj = getPeer(env, obj);
	
    if (maxObj == NULL) {
		error("mxj problem: unable to get peer in mxj_register_message");
		return;
	}
	
	m_name     = MXJ_JNI_CALL(env,GetStringUTFChars)(env, name, NULL);
	m_pt       = MXJ_JNI_CALL(env,GetStringUTFChars)(env, mxj_paramtypes, NULL);
	m_java_sig = MXJ_JNI_CALL(env,GetStringUTFChars)(env, java_sig, NULL);
	j_pt       = MXJ_JNI_CALL(env,GetStringUTFChars)(env, java_paramtypes, NULL);
	
	if (maxObj->cls == NULL) {
		error("mxj problem: javaclass in mxj_register_message is null for %s", m_name ? m_name : "<NULL>");
	}
	else {
		mid        = MXJ_JNI_CALL(env,GetMethodID)(env, maxObj->cls, m_name, m_java_sig);
		m = mxj_new_method((char*)m_name,(long)num_params,(char*)m_pt,(char*)j_pt,mid);
		checkException(env);
		mxj_methodlist_add(maxObj->methodlist,m);
	}
	
	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, name, m_name);
	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, mxj_paramtypes, m_pt);
	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, java_sig, m_java_sig);
	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, java_paramtypes, j_pt);
}

void init_max_object_callbacks(JNIEnv *env)
{
    g_max_object_clazz =  getClassByName(env, MXJ_MAX_OBJECT_CLASSNAME);
    mxj_max_object_register_natives(env,g_max_object_clazz);
    checkException(env);
}

void mxj_max_object_register_natives(JNIEnv *env,jclass clazz)
{
	int count=0;
	JNINativeMethod methods[] =
	{
   		{ "doPost", "([B)V", mxj_post },
   		{ "doError", "([B)V", mxj_error },
   		{ "doOuch", "([B)V", mxj_ouch },
   		{ "doOutlet","(II)Z", mxj_outlet_int },
   	    { "doOutlet","(IF)Z", mxj_outlet_float },
   	    { "doOutlet","(ILjava/lang/String;[Lcom/cycling74/max/Atom;)Z", mxj_outlet_anything },
   	    { "doOutlet","(I[Lcom/cycling74/max/Atom;)Z", mxj_outlet_list } ,
   	    { "doOutlet","(I[I)Z", mxj_outlet_list_int } ,
   	    { "doOutlet","(ILjava/lang/String;[I)Z", mxj_outlet_list_int_msg } ,
   	    { "doOutlet","(I[F)Z", mxj_outlet_list_float } ,
   	    { "doOutlet","(ILjava/lang/String;[F)Z", mxj_outlet_list_float_msg } ,
   	    { "doOutletBang","(I)Z", mxj_outlet_bang },
   	    { "doOutletHigh","(II)Z", mxj_outlet_int_high },
   	    { "doOutletHigh","(IF)Z", mxj_outlet_float_high },
   	    { "doOutletHigh","(ILjava/lang/String;[Lcom/cycling74/max/Atom;)Z", mxj_outlet_anything_high },
   	    { "doOutletHigh","(I[Lcom/cycling74/max/Atom;)Z", mxj_outlet_list_high } ,
   	    { "doOutletHigh","(I[I)Z", mxj_outlet_list_int_high } ,
   	    { "doOutletHigh","(ILjava/lang/String;[I)Z", mxj_outlet_list_int_msg_high } ,
   	    { "doOutletHigh","(I[F)Z", mxj_outlet_list_float_high } ,
   	    { "doOutletHigh","(ILjava/lang/String;[F)Z", mxj_outlet_list_float_msg_high } ,
   	    { "doOutletBangHigh","(I)Z", mxj_outlet_bang_high },
   	    { "getInlet", "()I", mxj_getinlet },
   	    { "_get_parent_patcher", "()J", mxj_get_parent_patcher },
   	    { "_get_max_box", "()J", mxj_get_max_box },
   	    { "_register_message", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",mxj_register_message},
   	    { "_register_attribute", "(Lcom/cycling74/max/AttributeInfo;)V",mxj_register_attribute},
   	  	{ "doEmbedMessage","(Ljava/lang/String;[Lcom/cycling74/max/Atom;)V", mxj_embed_message },
   	  	{ NULL, NULL, NULL }
	};
	
	while (methods[count].name) count++;
	
	MXJ_JNI_CALL(env,RegisterNatives)(env,clazz,methods,count);
	checkException(env);
}
