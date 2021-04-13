#ifndef _MXJ_ATTR_H
#define _MXJ_ATTR_H

#include "mxj_common.h"

#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif

typedef void (*mxj_attr_setter)(JNIEnv *env, void *x, void *a, short argc, t_atom *argv);
typedef void (*mxj_attr_getter)(JNIEnv *env, void *x, void *a, short *argc, t_atom **argv);

typedef struct mxj_attr
{
	jfieldID	 	fid;				// field id of member variable
	jmethodID       g_mid;				// method id of virtual attrib
	jmethodID       s_mid;				// method id of virtual attrib
	t_symbol	 	*name;
	t_symbol		*m_f_type;			// max field type
	t_symbol		*j_f_type;			// java field type
	t_symbol  		*setter_jptypes;	// setter java parameter types
	t_symbol  		*getter_jptypes;	// getter java parameter types
	
	char            name_setter[128];	// java setter name
	char            name_getter[128];	// java getter name
	mxj_attr_setter	setter; 
	mxj_attr_getter	getter;
	int             settable;
	int             gettable;
	int             isvirtual;
} t_mxj_attr;

//this is what comes back from java
typedef struct mxj_attr_desc
{
	char	*name;
	char	*m_f_type;			// max field type
	char	*j_f_type;			// java field type

	char	*setter;			// setter name
	char	*setter_jptypes;	// setter java_ptypes
	char	*setter_sig;		// setter java sig (so we can find methodid)

	char	*getter;			// getter name
	char	*getter_jptypes;	// getter java p_types (this is always ()[Lcom/cycling74/max/Atom for now)
	char	*getter_sig;		// getter java sig

	int		settable;
	int		gettable;
	int		isvirtual;
} t_mxj_attr_desc;

t_mxj_attr* mxj_attr_new(JNIEnv *env,jclass clazz,t_mxj_attr_desc* ad);
jvalue mxj_attr_get_val(JNIEnv* env, jobject x, t_mxj_attr *a);
long mxj_attr_val_as_long(JNIEnv* env, jobject x,t_mxj_attr *a);
double mxj_attr_val_as_double(JNIEnv* env, jobject x,t_mxj_attr *a);
t_symbol* mxj_attr_val_as_sym(JNIEnv* env, jobject x,t_mxj_attr *a);
void mxj_attr_set(JNIEnv *env, jobject obj, t_mxj_attr *a, t_atom *val);

void mxj_attr_free(t_mxj_attr* x);

#endif
