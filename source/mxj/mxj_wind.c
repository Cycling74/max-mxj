
#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif

#include "mxj_wind.h"
#include "mxj_utils.h"
#include "threadenv.h"
#include "classes.h"
#include "callbacks.h"
#include "ExceptionUtils.h"
#include "Atom.h"

#include "ext_obex.h"
#include "jpatcher_api.h"

#define MXJ_WIND_CLASSNAME "com/cycling74/max/MaxWindow"
#define MXJ_WIND_PTR_FIELD "_p_wind"

extern JavaVM *g_jvm;

static jfieldID  s_mxj_p_wind_fid;
static jclass    s_mxj_wind_clazz;

//JNI
void      JNICALL mxj_wind_set_visible(JNIEnv *env, jobject obj, jboolean b);
jboolean  JNICALL mxj_wind_get_visible(JNIEnv *env, jobject obj);
void      JNICALL mxj_wind_settitle(JNIEnv *env, jobject obj, jstring title);
jstring   JNICALL mxj_wind_gettitle(JNIEnv *env, jobject obj);
jboolean  JNICALL mxj_wind_hashorizscroll(JNIEnv *env, jobject obj);
jboolean  JNICALL mxj_wind_hasvertscroll(JNIEnv *env, jobject obj);
jboolean  JNICALL mxj_wind_haszoom(JNIEnv *env, jobject obj);
jboolean  JNICALL mxj_wind_hastitlebar(JNIEnv *env, jobject obj);
jboolean  JNICALL mxj_wind_hasclose(JNIEnv *env, jobject obj);
jboolean  JNICALL mxj_wind_hasgrow(JNIEnv *env, jobject obj);
jintArray JNICALL mxj_wind_getsize(JNIEnv *env, jobject obj);
jintArray JNICALL mxj_wind_getlocation(JNIEnv *env, jobject obj);
jlong     JNICALL mxj_wind_getassoc(JNIEnv *env, jobject obj);
jstring   JNICALL mxj_wind_getassocclass(JNIEnv *env, jobject obj);
jboolean  JNICALL mxj_wind_getdirty(JNIEnv *env, jobject obj);

void JNICALL mxj_wind_setzoom(JNIEnv *env, jobject obj, jboolean b);
void JNICALL mxj_wind_settitlebar(JNIEnv *env, jobject obj, jboolean b);
void JNICALL mxj_wind_setclose(JNIEnv *env, jobject obj, jboolean b);
void JNICALL mxj_wind_setgrow(JNIEnv *env, jobject obj, jboolean b);
void JNICALL mxj_wind_setfloat(JNIEnv *env, jobject obj, jboolean b);
void JNICALL mxj_wind_setsize(JNIEnv *env, jobject obj, jint width, jint height);
void JNICALL mxj_wind_setlocation(JNIEnv *env, jobject obj, jint x1, jint y1, jint x2, jint y2);
void JNICALL mxj_wind_setdirty(JNIEnv *env, jobject obj, jboolean b);
void JNICALL mxj_wind_close(JNIEnv *env, jobject obj);

//internal
t_object *get_mxj_wind(JNIEnv *env, jobject obj, t_object **p);
void mxj_wind_register_natives(JNIEnv*,jclass );
void mxj_wind_debracket(char *src, char *dst);
void mxj_windowmessage(t_object *p, char *mess1, char *mess2);


t_object *get_mxj_wind(JNIEnv *env, jobject obj, t_object **pa)
{
	t_object *w = NULL;
	t_object *p = (t_object *)(t_atom_long)(MXJ_JNI_CALL(env,GetLongField)(env, obj,s_mxj_p_wind_fid));
	
	if (p) {
		t_object *pv = jpatcher_get_firstview(p);

		if (pv)
			object_method(pv,gensym("getwind"), &w);
	}
	if (pa)
		*pa = p;
	
	return w;
}

void JNICALL mxj_wind_set_visible(JNIEnv *env, jobject obj, jboolean b)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj,&p);

	if (!w)
		return;
	// should be able to make a patcher visible eventually...
	object_attr_setchar(w, gensym("visible"), b);
}

jboolean JNICALL mxj_wind_get_visible(JNIEnv *env, jobject obj)
{
	t_object *w = get_mxj_wind(env,obj, NULL);
	return (jboolean)object_attr_getchar(w, gensym("visible"))? JNI_TRUE: JNI_FALSE;
}

void JNICALL mxj_wind_settitle(JNIEnv *env, jobject obj, jstring title)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	const char *t = MXJ_JNI_CALL(env,GetStringUTFChars)(env, title, NULL);
	
	if (t) {
		if (w)
			object_attr_setsym(w, gensym("title"), gensym(t));
		else if (p)
			object_attr_setsym(p, gensym("title"), gensym(t));
	}
	
    MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, title, t);
}

jstring JNICALL mxj_wind_gettitle(JNIEnv *env, jobject obj)
{
	t_symbol *title;
	t_object *w;
	char filename[512];
	
	w = get_mxj_wind(env,obj, NULL);

	if (w) {
		title = object_attr_getsym(w, gensym("title"));
		mxj_wind_debracket(title->s_name,filename);
		return MXJ_JNI_CALL(env,NewStringUTF)(env,filename);
	}
	
	return NULL;
}

jboolean JNICALL mxj_wind_hashorizscroll(JNIEnv *env, jobject obj)
{
	jboolean ret;
	t_object *w = get_mxj_wind(env,obj, NULL);
	
	ret = w && object_attr_getchar(w, gensym("hashorizscroll"));
	
	return ret;
}

jboolean JNICALL  mxj_wind_hasvertscroll(JNIEnv *env, jobject obj)
{
	jboolean ret;
	t_object *w = get_mxj_wind(env,obj, NULL);
	
	ret = w && object_attr_getchar(w, gensym("hasvertscroll"));
	
	return ret;
}

jboolean JNICALL  mxj_wind_haszoom(JNIEnv *env, jobject obj)
{
	jboolean ret;
	t_object *w = get_mxj_wind(env,obj, NULL);
	
	ret = w && object_attr_getchar(w, gensym("haszoom"));
	
	return ret;
}

jboolean JNICALL  mxj_wind_hastitlebar(JNIEnv *env, jobject obj)
{
	jboolean ret;
	t_object *w = get_mxj_wind(env,obj, NULL);
	
	ret = w && object_attr_getchar(w, gensym("hastitlebar"));
	
	return ret;
}

jboolean JNICALL  mxj_wind_hasclose(JNIEnv *env, jobject obj)
{
	jboolean ret;
	t_object *w = get_mxj_wind(env,obj, NULL);
	
	ret = w && object_attr_getchar(w, gensym("hasclose"));
	
	return ret;
}

jboolean JNICALL  mxj_wind_hasgrow(JNIEnv *env, jobject obj)
{
	jboolean ret;
	t_object *w = get_mxj_wind(env,obj, NULL);
	
	ret = w && object_attr_getchar(w, gensym("hasgrow"));
	
	return ret;
}

jintArray JNICALL mxj_wind_getsize(JNIEnv *env, jobject obj)
{
	jint tmp[2];
	jintArray iarr;
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	t_size size;
	
	if (w) {
		object_attr_getdouble_array(w, gensym("size"), 2, (double*) &size);
	} else if (p) {
		long argc = 4;
		t_atom argv[4];
		t_atom *argpv = argv;
		object_attr_getvalueof(p, gensym("defrect"), &argc, &argpv);
		if (argc) {
			size.width = atom_getfloat(argv + 2);
			size.height = atom_getfloat(argv + 3);
		} else {
			size.width = size.height = 0;
		}
	} else
		return NULL;
	
	iarr = MXJ_JNI_CALL(env,NewIntArray)(env,2);
	tmp[0] = (jint)size.width;
	tmp[1] = (jint)size.height;
	MXJ_JNI_CALL(env,SetIntArrayRegion)(env,iarr,0,2,tmp);
	
	return iarr;
}

jintArray JNICALL mxj_wind_getlocation(JNIEnv *env, jobject obj)
{
	jint tmp[4];
	jintArray iarr;
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	t_rect rect;
	
	if (w) {
		object_attr_getdouble_array(w, gensym("rect"), 4, (double*) &rect);
	} else if (p) {
		object_attr_getdouble_array(p, gensym("defrect"), 4, (double *)&rect);
	} else
		return NULL;
	
	tmp[0] = (jint)rect.x;
	tmp[1] = (jint)rect.y;
	tmp[2] = (jint)(rect.x+rect.width);
	tmp[3] = (jint)(rect.y+rect.height);
	iarr = MXJ_JNI_CALL(env,NewIntArray)(env,4);
	MXJ_JNI_CALL(env,SetIntArrayRegion)(env,iarr,0,4,tmp);
	
	return iarr;
}

jlong JNICALL mxj_wind_getassoc(JNIEnv *env, jobject obj)
{
	t_object *p = NULL;
	get_mxj_wind(env,obj, &p);
	
	return p_to_jlong(p);
}

jstring JNICALL mxj_wind_getassocclass(JNIEnv *env, jobject obj)
{
	return MXJ_JNI_CALL(env,NewStringUTF)(env,"patcher");
}

jboolean JNICALL  mxj_wind_getdirty(JNIEnv *env, jobject obj)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	
	if (w)
		return (jboolean)object_attr_getlong(w, gensym("dirty"));
	else if (p)
		return (jboolean)object_attr_getlong(p, gensym("dirty"));
	else
		return JNI_FALSE;
}


void JNICALL mxj_wind_setzoom(JNIEnv *env, jobject obj, jboolean b)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	
	if (w)
		object_attr_setchar(w, gensym("haszoom"), b? -1 : 0);
	else if (p)
		mxj_windowmessage(p, "flags", b? "zoom" : "nozoom");
}

void JNICALL mxj_wind_settitlebar(JNIEnv *env, jobject obj, jboolean b)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	
	if (w)
		object_attr_setchar(w, gensym("hastitlebar"), b? -1 : 0);
	else if (p)
		mxj_windowmessage(p, "flags", b? "title" : "notitle");
}

void JNICALL mxj_wind_setclose(JNIEnv *env, jobject obj, jboolean b)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	
	if (w)
		object_attr_setchar(w, gensym("hasclose"), b? -1 : 0);
	else if (p)
		mxj_windowmessage(p, "flags", b? "close" : "noclose");
}

void JNICALL mxj_wind_setgrow(JNIEnv *env, jobject obj, jboolean b)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	
	if (w)
		object_attr_setchar(w, gensym("hasgrow"), b? -1 : 0);
	else if (p)
		mxj_windowmessage(p, "flags", b? "grow" : "nogrow");
}

void JNICALL mxj_wind_setfloat(JNIEnv *env, jobject obj, jboolean b)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	
	if (w)
		object_attr_setchar(w, gensym("floating"), b? -1 : 0);
	else if (p)
		mxj_windowmessage(p, "flags", b? "float" : "nofloat");
}

void JNICALL mxj_wind_setsize(JNIEnv *env, jobject obj, jint width, jint height)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	t_rect rect;
	
	rect.width = width;
	rect.height = height;
	
	if (w)
		object_attr_setdouble_array(w, gensym("size"), 2, (double*) &rect.width);
	else if (p) {
		t_rect r2;
		
		object_attr_getdouble_array(w, gensym("defrect"), 4, (double *)&r2);
		rect.x = r2.x;
		rect.y = r2.y;
		object_attr_setdouble_array(w, gensym("defrect"), 4, (double *)&rect);
	}
}

void JNICALL mxj_wind_setlocation(JNIEnv *env, jobject obj, jint x1, jint y1, jint x2, jint y2)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	t_rect rect;
	
	rect.x = x1;
	rect.y = y1;
	rect.width = x2 - x1;
	rect.height = y2 - y1;

	if (w)
		object_attr_setdouble_array(w, gensym("rect"), 4, (double*) &rect);
	else if (p)
		object_attr_setdouble_array(p, gensym("defrect"), 4, (double *)&rect);
}

void JNICALL mxj_wind_setdirty(JNIEnv *env, jobject obj, jboolean b)
{
	t_object *p = NULL;
	t_object *w = get_mxj_wind(env,obj, &p);
	
	if (!p)
		return;
	
	object_attr_setchar(w? w : p, gensym("dirty"), b ? -1 : 0);
}

void JNICALL mxj_wind_close(JNIEnv *env, jobject obj)
{
	t_object *p = NULL;
	get_mxj_wind(env,obj, &p);
	
	if (p)
		object_method(p, gensym("wclose"));
}


void init_mxj_wind(JNIEnv *env)
{
	s_mxj_wind_clazz =  getClassByName(env, MXJ_WIND_CLASSNAME);
	checkException(env);
	s_mxj_p_wind_fid =  MXJ_JNI_CALL(env,GetFieldID)(env, s_mxj_wind_clazz, MXJ_WIND_PTR_FIELD, "J");
    checkException(env);
    mxj_wind_register_natives(env,s_mxj_wind_clazz);
    checkException(env);
}


void mxj_wind_register_natives(JNIEnv *env,jclass clazz)
{
	int count=0;
	JNINativeMethod methods[] =
	{
   		{ "setVisible","(Z)V", mxj_wind_set_visible },
   		{ "isVisible","()Z", mxj_wind_get_visible },
		{ "getTitle","()Ljava/lang/String;", mxj_wind_gettitle },
		{ "setTitle","(Ljava/lang/String;)V", mxj_wind_settitle },
		{ "hasHorizontalScroll","()Z",mxj_wind_hashorizscroll },
		{ "hasVerticalScroll","()Z",mxj_wind_hasvertscroll },
		{ "hasZoom","()Z",mxj_wind_haszoom },
		{ "hasTitleBar","()Z",mxj_wind_hastitlebar },
		{ "hasClose","()Z",mxj_wind_hasclose },
		{ "hasGrow","()Z",mxj_wind_hasgrow },
		{ "getSize","()[I",mxj_wind_getsize },
		{ "getLocation","()[I",mxj_wind_getlocation },
		{ "_get_patcher","()J",mxj_wind_getassoc },
		{ "getPatcherClass","()Ljava/lang/String;",mxj_wind_getassocclass },
		{ "isDirty","()Z",mxj_wind_getdirty },
		{ "setZoom","(Z)V",mxj_wind_setzoom },
		{ "setTitleBar","(Z)V",mxj_wind_settitlebar },
		{ "setClose","(Z)V",mxj_wind_setclose },
		{ "setGrow","(Z)V",mxj_wind_setgrow },
		{ "setFloat","(Z)V",mxj_wind_setfloat },
		{ "setSize","(II)V",mxj_wind_setsize },
		{ "setLocation","(IIII)V",mxj_wind_setlocation },
   		{ "setDirty","(Z)V",mxj_wind_setdirty },
   		{ "_close","()V",mxj_wind_close },
   		{ NULL, NULL, NULL }
	};
	
	while (methods[count].name) count++;
	MXJ_JNI_CALL(env,RegisterNatives)(env,clazz,methods,count);
	checkException(env);
}


//UTILS
void mxj_wind_debracket(char *src, char *dst)
{
	char *s,*p;
	
	strcpy(dst,src);
	s = dst;
	p = s;
	while (s[0]=='[') {
		while (*(s+1)) {
			*s = *(s+1);
			s++;
		}
		*(s-1) = 0;
		s = p;
	}
}

void mxj_windowmessage(t_object *p, char *mess1, char *mess2)
{
	long count = 1;
	t_atom argv[2];
	t_atom rv;
	
	if (mess2[0])
		count = 2;
	atom_setsym(argv,gensym(mess1));
	if (count == 2)
		atom_setsym(argv + 1,gensym(mess2));
	object_method_typed(p, gensym("window"), count, argv, &rv);
	atom_setsym(argv, gensym("exec"));
	object_method_typed(p, gensym("window"), 1, argv, &rv);
}

