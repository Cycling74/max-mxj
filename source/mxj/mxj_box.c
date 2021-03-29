
#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif

#include "mxj_box.h"
#include "mxj_utils.h"
#include "threadenv.h"
#include "classes.h"
#include "ExceptionUtils.h"
#include "Atom.h"
#include "ext.h"
#include "ext_obex.h"
#include "jpatcher_api.h"

#define MXJ_BOX_CLASSNAME "com/cycling74/max/MaxBox"
#define MXJ_BOX_PTR_FIELD "_p_box"

extern JavaVM *g_jvm;


static jfieldID  s_mxj_p_box_fid;
static jclass s_mxj_box_clazz;
static t_symbol *ps_bang,*ps_int,*ps_float, *ps_info;

//JNI
void    JNICALL mxj_box_free(JNIEnv *env, jobject obj);
void JNICALL mxj_box_set_rect(JNIEnv *env, jobject obj, jint x1, jint y1, jint x2, jint y2);
void JNICALL mxj_box_sethidden(JNIEnv *env, jobject obj,jboolean b);
void JNICALL mxj_box_set_color_index(JNIEnv *env, jobject obj, int colorindex);
void JNICALL mxj_box_to_background(JNIEnv *env, jobject obj, jboolean back);
void JNICALL mxj_box_ignore_click(JNIEnv *env, jobject obj, jboolean ignore);
void JNICALL mxj_box_set_name(JNIEnv *env, jobject obj, jstring name);
jboolean JNICALL mxj_box_send(JNIEnv *env, jobject obj,jstring msg,jobjectArray args);
jboolean JNICALL mxj_box_bang(JNIEnv *env, jobject obj);
jboolean JNICALL mxj_box_send_int(JNIEnv *env, jobject obj,jint n);
jboolean JNICALL mxj_box_send_float(JNIEnv *env, jobject obj,jfloat f);

jintArray JNICALL mxj_box_get_rect(JNIEnv *env, jobject obj);
jstring JNICALL mxj_box_get_class(JNIEnv  *env, jobject obj);
jboolean JNICALL mxj_box_get_hidden(JNIEnv  *env, jobject obj);
jint JNICALL mxj_box_get_color_index(JNIEnv  *env, jobject obj);
jlong JNICALL mxj_box_get_next_box(JNIEnv  *env, jobject obj);
jboolean JNICALL mxj_box_is_highlightable(JNIEnv  *env, jobject obj);
jboolean JNICALL mxj_box_in_background(JNIEnv  *env, jobject obj);
jboolean JNICALL mxj_box_is_selected(JNIEnv  *env, jobject obj);
jboolean JNICALL mxj_box_get_ignore_click(JNIEnv  *env, jobject obj);
jstring JNICALL mxj_box_get_name(JNIEnv  *env, jobject obj);
jboolean JNICALL mxj_box_understands(JNIEnv *env, jobject obj,jstring msg);
void JNICALL mxj_box_inspect(JNIEnv *env, jobject obj);
void JNICALL mxj_box_remove(JNIEnv *env, jobject obj);
jlong JNICALL mxj_box_get_subpatcher(JNIEnv *env, jobject obj);

//internal
t_object *get_mxj_box(JNIEnv *env, jobject obj);
void mxj_box_register_natives(JNIEnv*,jclass );
void mxj_box_redrawbox(t_object *b);


t_object *get_mxj_box(JNIEnv *env, jobject obj)
{
  return (t_object *)(t_atom_long)(MXJ_JNI_CALL(env,GetLongField)(env, obj,s_mxj_p_box_fid));
}

void JNICALL mxj_box_sethidden(JNIEnv *env, jobject obj, jboolean hidden)
{	
	t_object *b = get_mxj_box(env,obj);
	if (b) {
		jbox_set_hidden(b, hidden);
	}
}

void JNICALL mxj_box_set_color_index(JNIEnv *env, jobject obj, int colorindex)
{
	t_object *b = get_mxj_box(env,obj);
	if (b) {
		t_jrgba rgba; 
		set_jrgba_from_boxcolor_index(colorindex, &rgba); 
		jbox_set_color(b, &rgba); 
	}

}
void JNICALL mxj_box_to_background(JNIEnv *env, jobject obj, jboolean back)
{
	t_object *b = get_mxj_box(env,obj);
	if (b) {
		jbox_set_background(b, back ? 1 : 0); 
	}
}

void JNICALL mxj_box_ignore_click(JNIEnv *env, jobject obj, jboolean ignore)
{
	t_object *b = get_mxj_box(env,obj);
	if (b) {
		jbox_set_ignoreclick(b, ignore); 
	}
}

void JNICALL mxj_box_set_name(JNIEnv *env, jobject obj, jstring name)
{
	const char *s;
	t_object *b = get_mxj_box(env,obj);
	if (b) {
		s = MXJ_JNI_CALL(env,GetStringUTFChars)(env,name,NULL);
		jbox_set_varname(b, gensym((char *)s)); 
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,name,s);
	}
}

jboolean JNICALL mxj_box_bang(JNIEnv *env, jobject obj)
{
	t_object *maxobj;
	t_object *b = get_mxj_box(env,obj);
	
	if (b) {
		maxobj = jbox_get_object(b);
		mess0(maxobj,ps_bang);
		return JNI_TRUE;
	}

	return JNI_FALSE;	// shouldn't happen
}

jboolean JNICALL mxj_box_send_int(JNIEnv *env, jobject obj,jint n)
{
	t_object *maxobj;
	t_object *b = get_mxj_box(env,obj);
	
	if (b) {
		maxobj = jbox_get_object(b);
		object_method(maxobj, ps_int, (t_atom_long)n);
		return JNI_TRUE;
	}

	return JNI_FALSE;	// shouldn't happen
}
typedef void (*floatmeth)(t_object *x, double f);
jboolean JNICALL mxj_box_send_float(JNIEnv *env, jobject obj,jfloat f)
{
	floatmeth fm;
	t_object *maxobj;
	t_object *b = get_mxj_box(env,obj);
	
	if (b) {
		maxobj = jbox_get_object(b);
		fm = (floatmeth)egetfn(maxobj,ps_float);
		(*fm)(maxobj,f);

		return JNI_TRUE;
	}
	return JNI_FALSE;	// shouldn't happen

}

jboolean JNICALL mxj_box_send(JNIEnv *env, jobject obj,jstring msg,jobjectArray argv)
{
	char *s;
	short ac;
	t_atom *av;
	t_object *maxobj;
	void *res;
	t_object *b;
	
	if (!msg)
		return JNI_FALSE;
		
 	b = get_mxj_box(env,obj);
	if (!b)	// would be weird
		return JNI_FALSE;

	s = (char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env,msg,NULL);
	maxobj = jbox_get_object(b);
	
	if (argv != NULL) {
		av = newArgv(env,argv, &ac);
		res = typedmess(maxobj,gensym(s),ac,av);
		mxj_freebytes(av,ac * sizeof(t_atom));
	}
	else
		res = typedmess(maxobj,gensym(s),0,NULL);
	
	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,msg,s);
	
	if (res)
		return JNI_FALSE;
	else
		return JNI_TRUE;
}

void JNICALL mxj_box_free(JNIEnv *env, jobject obj)
{
	/* this doesn't need to do anything right now */
	
}

void JNICALL mxj_box_set_rect(JNIEnv *env, jobject obj, jint x1, jint y1, jint x2, jint y2)
{
	t_object *b = get_mxj_box(env,obj);
	if (b) {
		t_rect rect; 
		rect.x = x1; 
		rect.y = y1; 
		rect.width = x2 - x1;
		rect.height = y2 - y1; 

		// rbs -- assume patching rect for now
		jbox_set_patching_rect(b, &rect); 
	}
}

jintArray JNICALL mxj_box_get_rect(JNIEnv *env, jobject obj)
{
	jint tmp[4];
	jintArray iarr;
	t_rect rect; 

	t_object *b = get_mxj_box(env,obj);
	if (b) {
		jbox_get_patching_rect(b, &rect); 
		iarr = MXJ_JNI_CALL(env,NewIntArray)(env,4);
		tmp[0] = (jint)rect.x;
		tmp[1] = (jint)rect.y;
		tmp[2] = (jint)(rect.x + rect.width);
		tmp[3] = (jint)(rect.y + rect.height);
		MXJ_JNI_CALL(env,SetIntArrayRegion)(env,iarr,0,4,tmp);
		
		return iarr;
	}

	return NULL;
}

jstring JNICALL mxj_box_get_class(JNIEnv  *env, jobject obj)
{
	jstring ret;
	t_object *maxobj;
	t_object *b = get_mxj_box(env,obj);
	
	if (b) {
		maxobj = jbox_get_object(b);
		ret = MXJ_JNI_CALL(env,NewStringUTF)(env,ob_name(maxobj));
		return ret;
	}

	return NULL;
}

jboolean JNICALL mxj_box_get_hidden(JNIEnv  *env, jobject obj)
{
	jboolean ret;
	t_object *b = get_mxj_box(env,obj);

	if (b) {
		char hidden = jbox_get_hidden(b); 
		ret = hidden? JNI_TRUE : JNI_FALSE;
	
		return ret;
	}

	return JNI_FALSE;
}

jint JNICALL mxj_box_get_color_index(JNIEnv  *env, jobject obj)
{
	jint ret;
	t_object *b = get_mxj_box(env,obj);

	if (b) {
		t_jrgba rgba; 
		jbox_get_color(b, &rgba); 
		ret = get_boxcolor_index_from_jrgba(&rgba); 

		return ret;
	}

	return 0;
}

jlong JNICALL mxj_box_get_next_box(JNIEnv  *env, jobject obj)
{
	t_object *b2;
	t_object *b = get_mxj_box(env,obj);
	
	if (b) {
		b2 = jbox_get_nextobject(b);
		if (b2)
			return p_to_jlong(b2);

		return 0;
	}
	
	return 0;
}

jboolean JNICALL mxj_box_is_highlightable(JNIEnv  *env, jobject obj)
{
	jboolean ret;
	t_object *b = get_mxj_box(env,obj);

	if (b) {
		char hiliteable = jbox_get_canhilite(b);
		ret = hiliteable? JNI_TRUE : JNI_FALSE;

		return ret;
	}
	
	return JNI_FALSE;
}

jboolean JNICALL mxj_box_in_background(JNIEnv  *env, jobject obj)
{
	jboolean ret;
	t_object *b = get_mxj_box(env,obj);

	if (b) {
		char bg = jbox_get_background(b); 
		ret = bg ? JNI_TRUE : JNI_FALSE;
		return ret;
	}

	return JNI_FALSE;
}

jboolean JNICALL mxj_box_is_selected(JNIEnv  *env, jobject obj)
{
	jboolean ret;
	t_object *b = get_mxj_box(env,obj);

	if (b) {
		char selected = 0;		// rbs -- this doesn't make sense in max 5 -- selection needs view too
		ret = selected ? JNI_TRUE : JNI_FALSE;

		return ret;
	}
	
	return JNI_FALSE;
}

jboolean JNICALL mxj_box_get_ignore_click(JNIEnv  *env, jobject obj)
{
	jboolean ret;
	t_object *b = get_mxj_box(env,obj);

	if (b) {
		char ignoreclick = jbox_get_ignoreclick(b); 
		ret = ignoreclick ? JNI_TRUE : JNI_FALSE;

		return ret;
	}
	
	return JNI_FALSE;
}

jstring JNICALL mxj_box_get_name(JNIEnv  *env, jobject obj)
{
	jstring ret;
	t_object *b = get_mxj_box(env,obj);
	
	if (b) {
		t_symbol *varname = jbox_get_varname(b); 
		if (varname) {
			ret = MXJ_JNI_CALL(env,NewStringUTF)(env,varname->s_name);
			return ret;
		}

		return NULL;
	}
	
	return NULL;
}

jboolean JNICALL mxj_box_understands(JNIEnv *env, jobject obj,jstring msg)
{
	char *s;
	t_object *maxobj;
	jboolean res;
	t_object *b = get_mxj_box(env,obj);

	if (b) {	
		maxobj = jbox_get_object(b); 
		s = (char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env,msg,NULL);
		res = zgetfn(maxobj,gensym(s)) ? JNI_TRUE : JNI_FALSE;
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,msg,s);
		
		return res;
	}

	return JNI_FALSE;
}

void JNICALL mxj_box_inspect(JNIEnv *env, jobject obj)
{
	// not implemented at the moment
}

void JNICALL mxj_box_remove(JNIEnv *env, jobject obj)
{
	t_object *b = get_mxj_box(env,obj);
	if (b) {
		freeobject(b);
	}
	
	MXJ_JNI_CALL(env,SetLongField)(env, obj,s_mxj_p_box_fid,(jlong)0);	
}

jlong JNICALL mxj_box_get_subpatcher(JNIEnv *env, jobject obj)
{
	t_object *b = get_mxj_box(env,obj);
	t_object *p;
	t_object *maxobj;
	long index;
	
	if (b) {
		index = 0;
		maxobj = jbox_get_object(b); 
		p = (t_object *)object_subpatcher(maxobj, &index, 0);

		if (p) {
			return p_to_jlong(p);
		}
		
		return (jlong)0L;
	}
	
	return (jlong)0L;
}

void mxj_box_redrawbox(t_object *b)
{
	jbox_redraw((t_jbox*) b); 
}

void init_mxj_box(JNIEnv *env)
{
	s_mxj_box_clazz =  getClassByName(env, MXJ_BOX_CLASSNAME);
	checkException(env);
	s_mxj_p_box_fid =  MXJ_JNI_CALL(env,GetFieldID)(env, s_mxj_box_clazz, MXJ_BOX_PTR_FIELD, "J");
    checkException(env);
    mxj_box_register_natives(env,s_mxj_box_clazz);
    checkException(env);
    
    ps_bang = gensym("bang");
    ps_int  = gensym("int");
    ps_float = gensym("float");
    ps_info = gensym("info");
}

void mxj_box_register_natives(JNIEnv *env,jclass clazz)
{
	int count=0;

	JNINativeMethod methods[] =
	{
   		{ "_free","()V", mxj_box_free  },
   		{ "setHidden","(Z)V", mxj_box_sethidden },
   		{ "setColorIndex","(I)V", mxj_box_set_color_index },
   		{ "toBackground","(Z)V", mxj_box_to_background },
   		{ "setIgnoreClick","(Z)V", mxj_box_ignore_click },
   		{ "setName","(Ljava/lang/String;)V", mxj_box_set_name },
   		{ "bang","()Z",mxj_box_bang},
   		{ "send","(Ljava/lang/String;[Lcom/cycling74/max/Atom;)Z",mxj_box_send },
   		{ "send","(I)Z",mxj_box_send_int },
   		{ "send","(F)Z",mxj_box_send_float },
   
   		{ "setRect","(IIII)V",mxj_box_set_rect },
   		{ "getRect","()[I",mxj_box_get_rect },
   		{ "getMaxClass","()Ljava/lang/String;",mxj_box_get_class },
   		{ "getHidden","()Z",mxj_box_get_hidden },
   		{ "getColorIndex","()I", mxj_box_get_color_index },
   		{ "_get_next_box","()J",mxj_box_get_next_box },
   		{ "isHighlightable","()Z",mxj_box_is_highlightable},
   		{ "inBackground","()Z",mxj_box_in_background},
   		{ "isSelected","()Z",mxj_box_is_selected},
   		{ "getIgnoreClick","()Z",mxj_box_get_ignore_click},
   		{ "getName","()Ljava/lang/String;",mxj_box_get_name },
   		{ "understands","(Ljava/lang/String;)Z",mxj_box_understands},
   		{ "inspect","()V",mxj_box_inspect },
   		{ "remove","()V",mxj_box_remove },
   		{ "_get_subpatcher","()J",mxj_box_get_subpatcher },	
   		{ NULL, NULL, NULL } 
	};
   
	while (methods[count].name) count++;
	MXJ_JNI_CALL(env,RegisterNatives)(env,clazz,methods,count);
	checkException(env);
}
