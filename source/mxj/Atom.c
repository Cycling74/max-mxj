/*
 * Atom.c -- operations on com.cycling74.max.Atom objects.
 */

#include "mxj_macho_prefix.h"

#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif

#include "dbg.h"
#include "package.h"
#include "classes.h"
#include "Atom.h"
#include "mxj_utils.h"

extern jclass g_atomClass;
extern jmethodID g_newIntAtom_MID;
extern jmethodID g_newFloatAtom_MID;
extern jmethodID g_getAtomType_MID;
extern jmethodID g_getIntValue_MID;
extern jmethodID g_getFloatValue_MID;
extern jmethodID g_getStringValue_MID;
extern jmethodID g_newStringAtom_MID;

/*
 * Return a new Java Atom object containing the Max Atom's value.  Will create an
 * instance of IntAtom, FloatAtom, or StringAtom depending on the type of the Max Atom.
 */
jobject newAtom(JNIEnv *env, t_atom *a);

static jobjectArray g_zeroLengthAtomArray = NULL;

#define DEBUG FALSE

jobject newAtom(JNIEnv *env, t_atom *a) {
	jobject l_result = NULL;
	
    if (!a)
		return NULL;
	switch (a->a_type) {
		case A_LONG:
			l_result = MXJ_JNI_CALL(env,CallStaticObjectMethod)(env, g_atomClass,
												      g_newIntAtom_MID, (jint)a->a_w.w_long);
		    break;
				
		case A_FLOAT:
			l_result = MXJ_JNI_CALL(env,CallStaticObjectMethod)(env, g_atomClass,
												      g_newFloatAtom_MID, (jfloat)a->a_w.w_float);
		    break;
		        
	    case A_SYM: {
	    	jstring javaStr = MXJ_JNI_CALL(env,NewStringUTF)(env, a->a_w.w_sym->s_name);
	    	l_result = MXJ_JNI_CALL(env,CallStaticObjectMethod)(env, g_atomClass,
										    	      g_newStringAtom_MID,
													  javaStr);
			  //we can do this since now the atom object has a ref to the string 
    	      MXJ_JNI_CALL(env,DeleteLocalRef)(env,javaStr);
    	    break;
	    }
    	    
		default:
    		error("newAtom: default (%d)", a->a_type);
    		break;
	}
	
	return l_result;
}

jobjectArray newAtomArray(JNIEnv *env, short argc, t_atom *argv)
{
	jsize i;
	jobjectArray arr = MXJ_JNI_CALL(env,NewObjectArray)(env, argc, g_atomClass, NULL);
	
	for (i = 0; i < argc; i++) {
		jobject atom = newAtom(env, argv+i);
		MXJ_JNI_CALL(env,SetObjectArrayElement)(env, arr, i, atom);
		MXJ_JNI_CALL(env,DeleteLocalRef)(env,atom);
	}
	return arr;
}

jobjectArray zeroLengthAtomArray(JNIEnv *env)
{
	if (g_zeroLengthAtomArray == NULL) {
		jobject tmp;
		tmp = newAtomArray(env, 0, NULL);
		g_zeroLengthAtomArray = MXJ_JNI_CALL(env,NewGlobalRef)(env,tmp);
		MXJ_JNI_CALL(env,DeleteLocalRef)(env,tmp);
	}
	
	return g_zeroLengthAtomArray;
}

t_atom *newArgv(JNIEnv *env, jobjectArray array, short *nElts) {
	jsize argc;
	t_atom *result;
	int i;
	
	if(array == NULL) {
		*nElts = 0;
		return 0;
	}
	
	argc = MXJ_JNI_CALL(env,GetArrayLength)(env, array);
	result = (t_atom *)mxj_getbytes(argc * sizeof(t_atom));
		
	for (i = 0; i < argc; i++) {
		jobject javaAtom = MXJ_JNI_CALL(env,GetObjectArrayElement)(env, array, i);
		int javaAtomType = MXJ_JNI_CALL(env,CallIntMethod)(env, javaAtom, g_getAtomType_MID);

		switch (javaAtomType) {
			case ATOM_TYPENUM_LONG: {
				long value = MXJ_JNI_CALL(env,CallIntMethod)(env, javaAtom, g_getIntValue_MID);
				A_SETLONG(&result[i], value);
				break;
			}
			
			case ATOM_TYPENUM_FLOAT: {
				A_SETFLOAT(&result[i], MXJ_JNI_CALL(env,CallFloatMethod)(env, javaAtom, g_getFloatValue_MID));
			    break;
		    }
			    
		    case ATOM_TYPENUM_STRING: {
		    	jstring str = MXJ_JNI_CALL(env,CallObjectMethod)(env, javaAtom, g_getStringValue_MID);
		    	const char *chars = MXJ_JNI_CALL(env,GetStringUTFChars)(env, str, NULL);
		    	t_symbol *sym = gensym((char *)chars);
		    	A_SETSYM(&result[i], sym);
		    	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, str, chars);
		    	MXJ_JNI_CALL(env,DeleteLocalRef)(env,str);
		    	break;
	    	}
	    	
	    	default: {
	    		DBG(post("newArgv: default (%d)", javaAtomType));
	    		break;
			}
    	}
    	MXJ_JNI_CALL(env,DeleteLocalRef)(env, javaAtom);
	}
	(*nElts) = (short)argc;
	return result;
}

t_atom *newArgvFromIntArray(JNIEnv *env, jintArray array, short *nElts)
{
	jsize argc;
	t_atom *result;
	jint *ip;
	int i;
	
	if(array == NULL) {
		*nElts = 0;
		return 0;
	}
	
	argc = MXJ_JNI_CALL(env,GetArrayLength)(env, array);
	result = (t_atom *)mxj_getbytes(argc * sizeof(t_atom));
	ip = MXJ_JNI_CALL(env,GetPrimitiveArrayCritical)(env,array,0);
	
	for(i = 0;i < argc;i++)
		A_SETLONG(&result[i], ip[i]);
	
	MXJ_JNI_CALL(env,ReleasePrimitiveArrayCritical)(env,array,ip,0);	
	(*nElts) = (short)argc;
	
	return result;
}

t_atom *newArgvFromFloatArray(JNIEnv *env, jfloatArray array, short *nElts)
{
	jsize argc;
	t_atom *result;
	jfloat *fp;
	int i;
	
	if(array == NULL) {
		*nElts = 0;
		return 0;
	}
	
	argc = MXJ_JNI_CALL(env,GetArrayLength)(env, array);
	result = (t_atom *)mxj_getbytes(argc * sizeof(t_atom));
	fp = MXJ_JNI_CALL(env,GetPrimitiveArrayCritical)(env,array,0);
	
	for(i = 0;i < argc;i++)
		A_SETFLOAT(&result[i], fp[i]);
	
	MXJ_JNI_CALL(env,ReleasePrimitiveArrayCritical)(env,array,fp,0);
	(*nElts) = (short)argc;
	
	return result;
}

void dumpArgv(short argc, t_atom *argv)
{
	int i;
	
	for (i = 0; i < argc; i++) {
		switch (argv[i].a_type) {
			case A_LONG:
				post("(long) argv[%d]: %d", i, argv[i].a_w.w_long);
				break;
					
			case A_FLOAT:	
			    post("(float) argv[%d]: %f", i, argv[i].a_w.w_float);
		        break;
			        
		    case A_SYM:	
			    post("(sym)  argv[%d]: %s", i, argv[i].a_w.w_sym->s_name);
				break;
				
			default:
	    		post("dumpArgv: default (%d)", argv[i].a_type);
	    		break;
		}
	}
}

t_symbol *newSym(JNIEnv *env, jstring str)
{
	if(str) {
		const char *chars = MXJ_JNI_CALL(env,GetStringUTFChars)(env, str, NULL);
		t_symbol *sym = gensym((char *)chars);
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, str, chars);
		
		return sym;
		
	}
	else
		return NULL;
}
