#ifndef _MXJ_METHOD_H_
#define _MXJ_METHOD_H_

#include "mxj_common.h"

#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif

// In java method foo may have multiple implementations based on parameter types.
// This struct stores one method name with all its possible parameter types
// so we can call the appropriate java method, with coercion if neccessary, given a max message 
typedef struct _mxj_method
{
	t_symbol	 	*name;
	long            *p_argc;	// list of argc for each method
	t_symbol		**p_types;	// list of max parameter types
	t_symbol		**jp_types;	// list of java parameter types
	jmethodID	 	*mids;		// list of method ids
	long 			sigcount;
} t_mxj_method;

t_mxj_method *mxj_new_method(char *name,long p_argc, char *p_types, char* jp_types, jmethodID mid);
void mxj_method_add_sig(t_mxj_method *dest, t_mxj_method *src);
void mxj_method_free(t_mxj_method *m);

#endif //_MXJ_METHOD_H_
