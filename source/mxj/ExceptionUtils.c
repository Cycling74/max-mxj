
/*
 * ExceptionUtils - Utilities for dealing with exceptions.
 */

#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif

#include "mxj_macho_prefix.h"
#include "ExceptionUtils.h"

/*
 * Throw an exception by name.
 */
void JNU_ThrowByName(JNIEnv *env, const char *name, const char *msg)
{
    jclass cls = MXJ_JNI_CALL(env,FindClass)(env, name);
    /* if cls is NULL, exception was already thrown */
    if (cls != NULL) {
        MXJ_JNI_CALL(env,ThrowNew)(env, cls, msg);
    }
    /* free local ref */
    MXJ_JNI_CALL(env,DeleteLocalRef)(env, cls);
}

/*
 * Check for an exception having occurred.  Dump a description to System.err and clear the
 * exception.
 */
int checkException(JNIEnv *env)
{
    if (MXJ_JNI_CALL(env,ExceptionCheck)(env)) {
    	MXJ_JNI_CALL(env,ExceptionDescribe)(env);
		return 1;
	}
	return 0;
}
