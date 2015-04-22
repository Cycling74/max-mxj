
#ifdef MAC_VERSION
#include <JavaVM/jni.h>        // Java Native Interface definitions
#include <JavaVM/jni_md.h>
#else
#include "jni.h"
#include "jni_md.h"
#endif

#include "mxj_patcher.h"
#include "mxj_utils.h"
#include "threadenv.h"
#include "classes.h"
#include "callbacks.h"
#include "ExceptionUtils.h"
#include "Atom.h"
#include "ext_obex.h"

#define MXJ_PATCHER_CLASSNAME "com/cycling74/max/MaxPatcher"
#define MXJ_PATCHER_PTR_FIELD "_p_patcher"


typedef t_object t_patcherobject;
typedef t_object t_boxobject;

extern JavaVM *g_jvm;


jfieldID  s_mxj_p_patcher_fid;
jclass s_mxj_patcher_clazz;

//JNI
void    JNICALL mxj_patcher_free(JNIEnv *env, jobject obj);
jstring JNICALL mxj_patcher_get_path(JNIEnv *env, jobject obj);
void JNICALL mxj_patcher_settitle(JNIEnv *env, jobject obj, jstring title);
jstring JNICALL mxj_patcher_gettitle(JNIEnv *env, jobject obj);
void JNICALL mxj_patcher_set_brgb(JNIEnv *env, jobject obj, jint red, jint green, jint blue);
jboolean JNICALL mxj_isbpatcher(JNIEnv *env, jobject obj);
jlong JNICALL mxj_patcher_send_message(JNIEnv *env, jobject obj, jstring msg, jobject args);
void JNICALL mxj_patcher_setdirty(JNIEnv *env, jobject obj, jboolean b);
jlong JNICALL mxj_patcher_get_named_box(JNIEnv *env, jobject obj, jstring name);

jstring JNICALL mxj_patcher_get_name(JNIEnv *env, jobject obj);
jboolean JNICALL mxj_patcher_is_locked(JNIEnv *env, jobject obj);
jstring JNICALL mxj_patcher_parent_class(JNIEnv *env, jobject obj);
jintArray JNICALL mxj_patcher_get_offset(JNIEnv *env, jobject obj);
jintArray JNICALL mxj_patcher_get_origin(JNIEnv *env, jobject obj);
jint JNICALL mxj_patcher_get_count(JNIEnv *env, jobject obj);
jstring JNICALL mxj_patcher_get_filepath(JNIEnv *env, jobject obj);

void JNICALL mxj_patcher_set_locked(JNIEnv *env, jobject obj, jboolean locked);
jlong JNICALL mxj_patcher_new_default(JNIEnv *env, jobject obj,jint x, jint y, jstring maxclass, jobjectArray args);
void JNICALL mxj_patcher_connect(JNIEnv *env,jobject obj, jlong from_box_peer, jint outlet, jlong to_box_peer, jint inlet,jint color);
void JNICALL mxj_patcher_disconnect(JNIEnv *env,jobject obj, jlong from_box_peer, jint outlet, jlong to_box_peer, jint inlet); 
jlongArray JNICALL mxj_patcher_get_all_boxes(JNIEnv *env, jobject obj);
jlong JNICALL mxj_patcher_construct(JNIEnv *env, jobject obj, jint x1, jint y1, jint x2, jint y2);
jlong JNICALL mxj_patcher_get_window(JNIEnv *env, jobject obj);

jlong JNICALL mxj_patcher_new_object(JNIEnv *env, jobject obj, jstring msg, jobject args);
//internal
void mxj_patcher_typedmess_deferred(t_patcherobject* p,t_symbol *s,short argc, t_atom *argv);
t_patcherobject *get_mxj_patcher(JNIEnv *env, jobject obj);
void mxj_patcher_register_natives(JNIEnv*,jclass );

t_patcherobject *get_mxj_patcher(JNIEnv *env, jobject obj)
{
  return (t_patcherobject *)(t_atom_long)(MXJ_JNI_CALL(env,GetLongField)(env, obj,s_mxj_p_patcher_fid));
      
}

jlong JNICALL mxj_patcher_construct(JNIEnv *env, jobject obj, jint x1, jint y1, jint x2, jint y2)
{
	t_patcherobject *p;
	long top, left, bottom, right;
	t_atom av[4];
	
	// the arguments to a patcher are its location on the screen
	// then you set visible to true and give it a name using properties

	left   = x1;
	top    = y1;
	right  = x2;
	bottom = y2;
	
	A_SETLONG(av,left);
	A_SETLONG(av+1,top);
	A_SETLONG(av+2,right);
	A_SETLONG(av+3,bottom);
	
	p = newinstance(gensym("vpatcher"), 4, av);
	if (!p) {
		error("(mxj) MaxPatcher: can't create patcher");
		return (jlong)0L;
	}
	
	return p_to_jlong(p);
}

void JNICALL mxj_patcher_settitle(JNIEnv *env, jobject obj, jstring title)
{
	const char* t;
	
	t_patcherobject *p = get_mxj_patcher(env,obj);
	
	if (p) {
		t = MXJ_JNI_CALL(env,GetStringUTFChars)(env, title, NULL);
    	object_attr_setsym(p,gensym("title"),gensym((char *)t));
    	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, title, t);
	}
}

jstring JNICALL mxj_patcher_gettitle(JNIEnv *env, jobject obj)
{
	t_patcherobject *p = get_mxj_patcher(env,obj);
	
	if (p) {
		t_symbol *s = object_attr_getsym(p,gensym("title"));
		return MXJ_JNI_CALL(env,NewStringUTF)(env,s->s_name);
	}
	
	return NULL;
}

void JNICALL mxj_patcher_free(JNIEnv *env, jobject obj)
{
	/* this doesn't need to do anything right now since we are only
	    working with thispatcher */
}

jstring JNICALL mxj_patcher_get_path(JNIEnv *env, jobject obj)
{
	char pathname[MAX_PATH_CHARS];
	char native_pathname[MAX_PATH_CHARS];
	
	t_patcherobject *p = get_mxj_patcher(env,obj);
	
	if (p) {
		t_symbol *s = object_attr_getsym(p, gensym("pathname"));
		strcpy(pathname,s->s_name);
		path_nameconform(pathname,native_pathname,PATH_STYLE_MAX,PATH_TYPE_BOOT);
		return MXJ_JNI_CALL(env,NewStringUTF)(env,native_pathname);
	}
	
	return NULL;
}

void JNICALL mxj_patcher_set_brgb(JNIEnv *env, jobject obj, jint red, jint green, jint blue)
{
	t_patcherobject *p = get_mxj_patcher(env,obj);
	t_atom argv[3];	
	A_SETLONG((t_atom *)&argv[0],(long)red);
		A_SETLONG((t_atom *)&argv[1],(long)green);
		A_SETLONG((t_atom *)&argv[2],(long)blue);
	
	if (i_am_a_max_thread() && !i_am_in_java_constructor())
		typedmess((t_object*)p,gensym("brgb"),3,argv);
	else
		defer_low((t_object *)p,(method)mxj_patcher_typedmess_deferred,gensym("brgb"),3,argv);
}

void mxj_patcher_typedmess_deferred(t_patcherobject* p,t_symbol *s,short argc, t_atom *argv)
{
	typedmess((t_object *)p,s,argc,argv);
}

jboolean JNICALL mxj_isbpatcher(JNIEnv *env, jobject obj)
{
	t_patcherobject *p = get_mxj_patcher(env,obj);

	if (p) {
		t_boxobject *b = (t_boxobject *)object_attr_getobj((t_object *)p, gensym("box"));
		if (b && object_classname_compare(b, gensym("bpatcher")))
			return JNI_TRUE;
	}
	
	return JNI_FALSE;
}

jlong JNICALL mxj_patcher_get_named_box(JNIEnv *env, jobject obj, jstring name)
{
	const char *s;
	t_boxobject *b;
	t_patcherobject *p;
	
	if (!name)
		return 0L;
	
	p = get_mxj_patcher(env,obj);

	if (p) {
		s = MXJ_JNI_CALL(env,GetStringUTFChars)(env,name,NULL);
		b = (t_boxobject *)object_method(p, gensym("getnamedbox"), gensym((char *)s));		
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,name,s);
		return p_to_jlong(b);
	}
	
	return 0L;
}

jlongArray JNICALL mxj_patcher_get_all_boxes(JNIEnv *env, jobject obj)
{
	jlong *tmp;
	jlongArray larr;
	jsize cnt, i;
	t_boxobject *b;
	t_patcherobject *p = get_mxj_patcher(env,obj);

	if (p) {
		cnt = (jsize)object_attr_getlong(p, gensym("count"));
		tmp = (jlong*)mxj_getbytes(cnt * sizeof(jlong));

		i = 0;
		b = object_attr_getobj(p, gensym("firstobject"));
		while (b && i < cnt) {
			tmp[i++] = p_to_jlong(b);
			b = object_attr_getobj(b, gensym("nextobject"));
		}
		
		larr = MXJ_JNI_CALL(env,NewLongArray)(env,cnt);			
		MXJ_JNI_CALL(env,SetLongArrayRegion)(env,larr,0,cnt,tmp);
		mxj_freebytes(tmp, (cnt * sizeof(jlong)));
		
		return larr;
	}

	return NULL;
}

jlong JNICALL mxj_patcher_send_message(JNIEnv *env, jobject obj, jstring msg, jobject args)
{
	const char *s;
	short argc = 0;
	t_atom *argv = NULL;
	jlong ret;
	t_patcherobject *p = get_mxj_patcher(env,obj);

	if (p && msg) {
		if (args)
			argv = newArgv(env, args, &argc);
		s = MXJ_JNI_CALL(env,GetStringUTFChars)(env,msg,NULL);

		if (i_am_a_max_thread() && !i_am_in_java_constructor())
			ret = p_to_jlong(typedmess((t_object*)p,gensym((char*)s),(short)argc,argv));
		else {
			defer_low((t_object *)p,(method)mxj_patcher_typedmess_deferred,gensym((char *)s),argc,argv);
			ret = 0;
		}
		
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,msg,s);

		return ret;
	}
	return -1;
}

jstring JNICALL mxj_patcher_get_name(JNIEnv *env, jobject obj)
{	
	t_patcherobject *p = get_mxj_patcher(env,obj);
	if (p) {
		t_symbol *s = object_attr_getsym(p,gensym("name"));

		return MXJ_JNI_CALL(env,NewStringUTF)(env, s->s_name);
	}
	
	return NULL;
}

jboolean JNICALL mxj_patcher_is_locked(JNIEnv *env, jobject obj)
{
	jboolean res;

	t_patcherobject *p = get_mxj_patcher(env,obj);
	if (p) {
		res = object_attr_getchar(p, gensym("locked")) ? JNI_TRUE : JNI_FALSE;

		return res;
	}
	
	return JNI_FALSE;
}

jstring JNICALL mxj_patcher_parent_class(JNIEnv *env, jobject obj)
{
	t_patcherobject *p = get_mxj_patcher(env,obj);

	if (p) {
		t_symbol *s = object_attr_getsym(p, gensym("parentclass"));
		if (s && s!=gensym(""))
			return MXJ_JNI_CALL(env,NewStringUTF)(env, s->s_name);
	}

	return NULL;
}

jintArray JNICALL mxj_patcher_get_offset(JNIEnv *env, jobject obj)
{
	jint tmp[2];
	jintArray iarr;

	t_patcherobject *p = get_mxj_patcher(env,obj);
	
	if (p) {
		iarr = MXJ_JNI_CALL(env,NewIntArray)(env,2);
		// TODO - from what I can tell there is currently no way to get at the
		// scroll attributes of a window. for now I'm just going to populate
		// the array with zeroes.
		tmp[0] = 0;
		tmp[1] = 0;	
		post("mxj_patcher_get_offset: not supported");
		MXJ_JNI_CALL(env,SetIntArrayRegion)(env,iarr,0,2,tmp);

		return iarr;
	}

	return NULL;
}

jintArray JNICALL mxj_patcher_get_origin(JNIEnv *env, jobject obj)
{
	jint tmp[2];
	jintArray iarr;

	t_patcherobject *p = get_mxj_patcher(env,obj);
	if (p) {
		iarr = MXJ_JNI_CALL(env,NewIntArray)(env,2);
		// TODO - same as above.  scrollorigin does not seem to be present.
		// scroll attributes of a window. for now I'm just going to populate
		// the array with zeroes.	
		tmp[0] = 0;
		tmp[1] = 0;	
		post("mxj_patcher_get_origin: not supported");
		MXJ_JNI_CALL(env,SetIntArrayRegion)(env,iarr,0,2,tmp);

		return iarr;
	}
	
	return NULL;
}

jint JNICALL mxj_patcher_get_count(JNIEnv *env, jobject obj)
{
	t_patcherobject *p = get_mxj_patcher(env,obj);
	if (p) {
		return (jint)object_attr_getlong(p, gensym("count"));
	}

	return 0;
}

jstring JNICALL mxj_patcher_get_filepath(JNIEnv *env, jobject obj)
{
	char pathname[MAX_PATH_CHARS];
	char native_pathname[MAX_PATH_CHARS];
	
	t_patcherobject *p = get_mxj_patcher(env,obj);

	if (p) {
		t_symbol *s = object_attr_getsym(p, gensym("filepath"));
		strcpy(pathname,s->s_name);
		path_nameconform(pathname,native_pathname,PATH_STYLE_MAX,PATH_TYPE_BOOT); // This might be slightly different than the old style returned (should use absolute?) - jkc
		return MXJ_JNI_CALL(env,NewStringUTF)(env,native_pathname);
	}
	
	return NULL;
}

void JNICALL mxj_patcher_set_locked(JNIEnv *env, jobject obj, jboolean locked)
{	
	t_patcherobject *p = get_mxj_patcher(env,obj);
	
	if (p) {
		object_attr_setchar(p, gensym("locked"),locked?1:0);
	}
}

jlong JNICALL mxj_patcher_new_default(JNIEnv *env, jobject obj,jint x, jint y, jstring maxclass, jobjectArray args)
{
	t_patcherobject *p;
	t_boxobject *b;
	short ac;
	t_atom *av;
	t_symbol *sym;
	long ac2 = 0;
	t_atom *av2 = NULL;
	int i;
	char *s;
	char *textstr = 0;
	long textsize = 0;
	
	p = get_mxj_patcher(env,obj);

	if (!p || !maxclass)
		return 0;
		
	s = (char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env,maxclass,NULL);
	
	sym = gensym(s);
	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,maxclass,s);
	
	if (args) {
		av = newArgv(env,args, &ac);
		ac2 = ac + 1;
		av2 = (t_atom *)sysmem_newptr(ac2 * sizeof(t_atom));

		//copy args
		for(i = 0; i < ac;i++)
			av2[i+1] = av[i];
		mxj_freebytes(av,ac * sizeof(t_atom));
	} else {
		ac2 = 1;
		av2 = (t_atom *)sysmem_newptr(ac2 * sizeof(t_atom));
	}
	
	atom_setsym(av2, sym);
	
	atom_gettext(ac2, av2, &textsize, &textstr, 0);
	
	if (textstr) {
		if (strchr(textstr,' '))
			b = newobject_sprintf(p, "@maxclass newobj @patching_position %ld %ld @text \"%s\"",x,y,textstr);
		else
			b = newobject_sprintf(p, "@maxclass newobj @patching_position %ld %ld @text %s",x,y,textstr);
		sysmem_freeptr(textstr);
	} else
		b = newobject_sprintf(p, "@maxclass newobj @patching_position %ld %ld",x,y);
	
	sysmem_freeptr(av2);
	
	return p_to_jlong(b);
}

jlong JNICALL mxj_patcher_new_object(JNIEnv *env, jobject obj, jstring msg, jobject args)
{	
	const char *s;
	short argc;
	t_atom *argv;		
	jlong ret;
	t_patcherobject *p = get_mxj_patcher(env,obj);
	argc = 0;

	if (!p)
		return 0;
	
	if (p && msg && args) {
		if (args)
			argv = newArgv(env, args, &argc);
		
		if (i_am_a_max_thread() && !i_am_in_java_constructor()) {
			s = MXJ_JNI_CALL(env,GetStringUTFChars)(env,msg,NULL);
			ret = p_to_jlong(typedmess((t_object*)p,gensym((char*)s),(short)argc,argv));
			MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,msg,s);

			return ret;
		}
		
		return 0;
	}
	
	return 0;
}

void JNICALL mxj_patcher_connect(JNIEnv *env,jobject obj, jlong from_box_peer, jint outlet, jlong to_box_peer, jint inlet, jint color)
{
	t_boxobject *b1,*b2;
	t_atom a[5];
	
	t_patcherobject *p = get_mxj_patcher(env,obj);
	if (!p)
		return;
			
	b1 = (t_boxobject*)(t_atom_long)from_box_peer;
	if (!b1)
		return;
	
	b2 = (t_boxobject*)(t_atom_long)to_box_peer;;
	if (!b2)
		return;
	
	atom_setobj(a, b1);
	atom_setlong(a+1, (long)outlet);
	atom_setobj(a+2, b2);
	atom_setlong(a+3, (long)inlet);
	atom_setlong(a+4, (long)color);
	
	object_method(p, gensym("connect"), NULL, 5, a);
}

void JNICALL mxj_patcher_disconnect(JNIEnv *env,jobject obj, jlong from_box_peer, jint outlet, jlong to_box_peer, jint inlet)
{
	t_boxobject *b1,*b2;
	t_atom a[4];
	
	t_patcherobject *p = get_mxj_patcher(env,obj);
	if (!p)
		return;
			
	b1 = (t_boxobject*)(t_atom_long)from_box_peer;
	if (!b1)
		return;
	b2 = (t_boxobject*)(t_atom_long)to_box_peer;;
	if (!b2)
		return;

	atom_setobj(a, b1);
	atom_setlong(a+1, (long)outlet);
	atom_setobj(a+2, b2);
	atom_setlong(a+3, (long)inlet);
	
	object_method(p, gensym("disconnect"), NULL, 5, a);
} 

jlong JNICALL mxj_patcher_get_window(JNIEnv *env, jobject obj)
{
	t_patcherobject *p = get_mxj_patcher(env,obj);
	if (!p)
		return 0;	
	
	return p_to_jlong(p); // we use the patcher for creating windows now -jkc
}

void init_mxj_patcher(JNIEnv *env)
{
	s_mxj_patcher_clazz =  getClassByName(env, MXJ_PATCHER_CLASSNAME);
	checkException(env);
	s_mxj_p_patcher_fid =  MXJ_JNI_CALL(env,GetFieldID)(env, s_mxj_patcher_clazz, MXJ_PATCHER_PTR_FIELD, "J");
    checkException(env);
    mxj_patcher_register_natives(env,s_mxj_patcher_clazz);
    checkException(env);
}

void mxj_patcher_register_natives(JNIEnv *env,jclass clazz)
{
   int count=0;
   JNINativeMethod methods[] = 
   {
   		{ "_free","()V", mxj_patcher_free  },
   		{ "getPath","()Ljava/lang/String;", mxj_patcher_get_path },
   		{ "setBackgroundColor","(III)V",mxj_patcher_set_brgb},
   		{ "isBPatcher","()Z",mxj_isbpatcher},
   		{ "send","(Ljava/lang/String;[Lcom/cycling74/max/Atom;)J",mxj_patcher_send_message},
		{ "_get_named_box","(Ljava/lang/String;)J",mxj_patcher_get_named_box},
		{ "getName","()Ljava/lang/String;",mxj_patcher_get_name },
		{ "isLocked","()Z",mxj_patcher_is_locked},
		{ "getParentMaxClass","()Ljava/lang/String;",mxj_patcher_parent_class },
		{ "getOffset","()[I",mxj_patcher_get_offset},
		{ "getOrigin","()[I",mxj_patcher_get_origin},
		{ "getCount","()I",mxj_patcher_get_count},
		{ "getFilePath","()Ljava/lang/String;", mxj_patcher_get_filepath },
		{ "setLocked","(Z)V",mxj_patcher_set_locked},
		{ "_new_default","(IILjava/lang/String;[Lcom/cycling74/max/Atom;)J",mxj_patcher_new_default},
		{ "_new_object","(Ljava/lang/String;[Lcom/cycling74/max/Atom;)J",mxj_patcher_new_object},
		{ "_connect","(JIJII)V",mxj_patcher_connect},
		{ "_disconnect","(JIJI)V",mxj_patcher_disconnect},
		{ "_get_all_boxes","()[J",mxj_patcher_get_all_boxes },
		{ "_patcher_construct","(IIII)J",mxj_patcher_construct },
		{ "_get_window_ptr","()J", mxj_patcher_get_window },
   		{ NULL, NULL, NULL } 
   };
   
   while (methods[count].name) count++; 
   MXJ_JNI_CALL(env,RegisterNatives)(env,clazz,methods,count);
   checkException(env);  
}
