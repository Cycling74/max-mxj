
#ifdef MAC_VERSION
#include <JavaVM/jni.h>        // Java Native Interface definitions
#include <JavaVM/jni_md.h>
#else
#include "jni.h"
#include "jni_md.h"
#endif

#include "ext_sysmem.h"
#include "maxjava.h"
#include "threadenv.h"
#include "classes.h"
#include "ExceptionUtils.h"
#include "mxj_max_system.h"
#include "mxj_utils.h"
#include "ext_path.h"
#include "Atom.h"
#include "ext_obex.h"
//#include "appstatus.h"
//#include "appstatus.c"
#include "callbacks.h"

#define MXJ_EXEC_METHOD            "execute"
#define MXJ_EXECUTABLE_CLASSNAME   "com/cycling74/max/Executable"
#define MXJ_MAXSYS_CLASSNAME       "com/cycling74/max/MaxSystem"

short i_am_a_max_thread(void);

extern jclass g_stringClass;

typedef struct t_exec_wrapper {
	t_object ob;
	jobject executable;
	JNIEnv *env;
} t_exec_wrapper;

extern JavaVM *g_jvm;

static jmethodID s_exec_mid;
static jclass    s_exec_clazz;
static jclass    s_mxjsys_clazz;
static void		*s_mxj_exec_wrapper_class=NULL; //max class

void JNICALL mxj_defer(JNIEnv *, jclass, jobject);
void JNICALL mxj_defer_low(JNIEnv *, jclass, jobject);
void JNICALL mxj_defer_medium(JNIEnv *, jclass, jobject);
void JNICALL mxj_defer_front(JNIEnv *, jclass, jobject);
void JNICALL mxj_schedule(JNIEnv *, jclass, jobject, jdouble);
void JNICALL mxj_schedule_delay(JNIEnv *, jclass, jobject, jdouble);
void JNICALL mxj_schedule_defer(JNIEnv *, jclass, jobject, jdouble);

void JNICALL mxj_hide_cursor(JNIEnv *env, jclass clazz);
void JNICALL mxj_show_cursor(JNIEnv *env, jclass clazz);

//PATH STUFF
jstring JNICALL mxj_open_dialog(JNIEnv *, jclass, jstring);
jstring JNICALL mxj_saveas_dialog(JNIEnv *, jclass, jstring,jstring);
jstring JNICALL mxj_locate_file(JNIEnv *,jclass, jstring);
jstring JNICALL mxj_nameconform(JNIEnv *env, jclass clazz,jstring jpath, jint style, jint type);
jstring JNICALL mxj_maxpath_to_native_path(JNIEnv *env,jclass clazz, jstring max_path);
jstring JNICALL mxj_get_preferences_path(JNIEnv *env, jclass clazz);
jstring JNICALL mxj_get_default_path(JNIEnv *env,jclass clazz);
jobjectArray JNICALL mxj_get_search_path(JNIEnv *env,jclass clazz);
jobjectArray JNICALL mxj_get_search_path_forcontext(JNIEnv *env,jclass clazz);

jboolean JNICALL mxj_in_max_thread(JNIEnv *, jclass);
jboolean JNICALL mxj_in_timer_thread(JNIEnv *, jclass);
jboolean JNICALL mxj_in_main_thread(JNIEnv *, jclass);

//workaround for swing bug
void JNICALL mxj_next_window_is_modal(JNIEnv *env,jclass clazz);

jshort JNICALL mxj_get_max_version(JNIEnv*,jclass);
jboolean JNICALL mxj_get_is_runtime(JNIEnv*,jclass cls);

void JNICALL mxj_system_loadobject(JNIEnv *env,  jclass cls, jbyteArray bytes);

void JNICALL mxj_system_error(JNIEnv *env, jclass cls, jbyteArray bytes);
void JNICALL mxj_system_ouch(JNIEnv *env, jclass cls, jbyteArray bytes);
void JNICALL mxj_system_post(JNIEnv *env, jclass cls, jbyteArray bytes);

//allows java to send messages to objects bound to a global symbol via gensym.
//request from IRCAM so they can use boxless mxj editor classes for their FTMlib
jboolean JNICALL mxj_send_message_to_bound_object(JNIEnv *env, jclass clz, jstring name, jstring msg,jobjectArray argv);
void mxj_exec_wrapper_init();
t_exec_wrapper *mxj_exec_wrapper_new(JNIEnv *env, jobject obj);
void mxj_exec_wrapper_free(JNIEnv *env,t_exec_wrapper *x);
void mxj_fn(t_exec_wrapper *client, t_symbol *s, short argc,t_atom *argv);

void mxj_max_system_register_natives(JNIEnv* env,jclass clazz );

void JNICALL mxj_register_accelerator(JNIEnv* env, jobject obj,jchar key );
void JNICALL mxj_unregister_accelerator(JNIEnv* env, jobject obj,jchar key );

//allows java windows to 'steal' cmnd key accelerators when java window is in focus
//..for now it does nothing on windows since I don't think we need it.
void mxj_add_acc_binding(short key);
void mxj_remove_acc_binding(short key);
jdouble JNICALL mxj_systimer_gettime(JNIEnv *env, jclass clz);


void mxj_fn(t_exec_wrapper *x, t_symbol *s, short argc,t_atom *argv)
{
	THREADENV(x->env);
	
	MXJ_JNI_CALL(x->env,CallVoidMethod)(x->env, x->executable, s_exec_mid);
    checkException(x->env);
 	mxj_exec_wrapper_free(x->env,x);
}

void JNICALL mxj_defer(JNIEnv *env, jclass clazz, jobject obj)
{
	t_exec_wrapper *x;
	
	x = mxj_exec_wrapper_new(env,obj);
	defer((t_object *)x,(method)mxj_fn,(t_symbol *) 0L, (short)0L, (t_atom *)0L);
}

void JNICALL mxj_defer_low(JNIEnv *env, jclass clazz, jobject obj)
{
	t_exec_wrapper *x;
	x = mxj_exec_wrapper_new(env,obj);
	defer_low((t_object *)x,(method)mxj_fn,(t_symbol *) 0L, (short)0L, (t_atom *)0L);
}

void JNICALL mxj_defer_medium(JNIEnv *env, jclass clazz, jobject obj)
{
	t_exec_wrapper *x;
	x = mxj_exec_wrapper_new(env,obj);
	defer_medium((t_object *)x,(method)mxj_fn,(t_symbol *) 0L, (short)0L, (t_atom *)0L);
}

void JNICALL mxj_defer_front(JNIEnv *env, jclass clazz, jobject obj)
{
	t_exec_wrapper *x;
	x = mxj_exec_wrapper_new(env,obj);
	defer_front((t_object *)x,(method)mxj_fn,(t_symbol *) 0L, (short)0L, (t_atom *)0L);
}


void JNICALL mxj_schedule(JNIEnv *env, jclass clazz, jobject obj, jdouble time)
{
	t_exec_wrapper *x;
	x = mxj_exec_wrapper_new(env,obj);
	schedulef((t_object *)x,(method)mxj_fn,(double)time,(t_symbol *) 0L, (short)0L, (t_atom *)0L);
	
}
void JNICALL mxj_schedule_delay(JNIEnv *env, jclass clazz, jobject obj, jdouble delay)
{
	t_exec_wrapper *x;
	x = mxj_exec_wrapper_new(env,obj);
	schedule_fdelay((t_object *)x,(method)mxj_fn,(double)delay,(t_symbol *) 0L, (short)0L, (t_atom *)0L);
}

void JNICALL mxj_schedule_defer(JNIEnv *env, jclass clazz, jobject obj, jdouble delay)
{
	t_exec_wrapper *x;
	x = mxj_exec_wrapper_new(env,obj);
	schedule_fdefer((t_object *)x,(method)mxj_fn,(double)delay,(t_symbol *) 0L, (short)0L, (t_atom *)0L);
}

jboolean JNICALL mxj_in_max_thread(JNIEnv* env, jobject obj)
{
	return (jboolean) i_am_a_max_thread();
}

jboolean JNICALL mxj_in_timer_thread(JNIEnv* env, jobject obj)
{
	return (jboolean)systhread_istimerthread();
}

jboolean JNICALL mxj_in_main_thread(JNIEnv* env, jobject obj)
{
	return (jboolean)systhread_ismainthread();
}

void mxj_exec_wrapper_init()
{
	t_class *c = class_findbyname(gensym("nobox"),gensym("mxj_exec_wrapper"));
	
	if (!c) {
		c = class_new("mxj_exec_wrapper",(method)mxj_exec_wrapper_new,(method)NULL,sizeof(t_exec_wrapper),(method)NULL,A_CANT,0L);
		class_register(gensym("nobox"),c);
	}
	
	s_mxj_exec_wrapper_class = c;
}

void mxj_exec_wrapper_free(JNIEnv* env,t_exec_wrapper *x)
{
	if (x) {
		MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->executable);
		checkException(env);
   		freeobject((t_object *)x);
	}
}

t_exec_wrapper *mxj_exec_wrapper_new(JNIEnv *env, jobject obj)
{
	t_exec_wrapper *x;
	
	x = (t_exec_wrapper*)object_alloc(s_mxj_exec_wrapper_class);
	x->executable = MXJ_JNI_CALL(env,NewGlobalRef)(env,obj);
	x->env = env;
	
	return x;
}

void init_mxj_max_system(JNIEnv *env)
{
	mxj_exec_wrapper_init();
	s_mxjsys_clazz   =  getClassByName(env, MXJ_MAXSYS_CLASSNAME);
	s_exec_clazz         =  getClassByName(env, MXJ_EXECUTABLE_CLASSNAME);
	s_exec_mid			 =  MXJ_JNI_CALL(env,GetMethodID)(env,s_exec_clazz,MXJ_EXEC_METHOD,"()V");
	checkException(env);
    mxj_max_system_register_natives(env,s_mxjsys_clazz);
    checkException(env);
}

void JNICALL mxj_hide_cursor(JNIEnv *env, jclass clazz)
{
#ifdef MAC_VERSION
	HideCursor();
#else
	ShowCursor(FALSE);
#endif
}

void JNICALL mxj_show_cursor(JNIEnv *env, jclass clazz)
{
#ifdef MAC_VERSION
	ShowCursor();
#else
	ShowCursor(TRUE);
#endif
}

jstring JNICALL mxj_open_dialog(JNIEnv *env,jclass clazz,jstring prompt)
{
	const char* pr;
	short res;
	short path;
	char filename[MAX_PATH_CHARS];
	char pathname[MAX_PATH_CHARS];
	char native_pathname[MAX_PATH_CHARS];
	t_fourcc *types;
	t_fourcc dsttype;
	
	if (prompt) {
		pr = MXJ_JNI_CALL(env,GetStringUTFChars)(env,prompt,NULL);
		open_promptset((char *)pr);
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,prompt,pr);
	}
	
	types = 0;
	res = open_dialog(filename,&path,&dsttype,types,0);	// ignoring all type information.let user open all types always.
	
	if (!res) {
		path_topathname(path, filename, pathname);
		max_path_to_native_path(pathname,native_pathname);
		
		return MXJ_JNI_CALL(env,NewStringUTF)(env,native_pathname);
	}
	
	return NULL;
}


jstring JNICALL mxj_saveas_dialog(JNIEnv *env,jclass clazz,jstring prompt,jstring default_filename)
{
	const char* pr;
	const char* dfn;
	short res;
	short path;
	
	t_fourcc typelist;
	t_fourcc type;
	char filename[256];
	char pathname[512];
	char native_pathname[512];
	pathname[0] = '\0';
	native_pathname[0] = '\0';
	
	if (prompt) {
		pr = MXJ_JNI_CALL(env,GetStringUTFChars)(env,prompt,NULL);
		saveas_promptset((char *)pr);
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,prompt,pr);
	}
	
	if (default_filename) {
		dfn = MXJ_JNI_CALL(env,GetStringUTFChars)(env,default_filename,NULL);
		strcpy(filename,dfn);
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,default_filename,dfn);
	}
	
	res = saveasdialog_extended(filename,&path,&type,&typelist,0);
	
	if (!res) {
		path_topotentialname(path, filename, pathname,0);
		max_path_to_native_path(pathname,native_pathname);
		return MXJ_JNI_CALL(env,NewStringUTF)(env, native_pathname);
	}
	
	return NULL;
}

jstring JNICALL mxj_locate_file(JNIEnv *env,jclass clazz, jstring jfilename)
{
	char *filename;
	short path;
	t_fourcc typelist;
	t_fourcc outtype;
	short res;
	char pathname[MAX_PATH_CHARS];
	char native_pathname[MAX_PATH_CHARS];
	
	if (jfilename) {
		filename = (char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env,jfilename,NULL);
		sprintf(native_pathname, "%s", filename); // also using native_pathname as a temporary string
		res = locatefile_extended(native_pathname,&path,&outtype,&typelist,0);
		
		if (!res) {
			path_topathname(path, native_pathname, pathname);
			MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,jfilename,filename);
			max_path_to_native_path(pathname,native_pathname);
			return MXJ_JNI_CALL(env,NewStringUTF)(env,native_pathname);
		}
		
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,jfilename,filename);
		
		return NULL;
	}

	return NULL;
}

jstring JNICALL mxj_get_default_path(JNIEnv *env,jclass clazz)
{
	short res;
	char pathname[MAX_PATH_CHARS];
	char native_pathname[MAX_PATH_CHARS];
	
	res = path_getdefault();
	if (res) {
		path_topathname(res,"", pathname);
		max_path_to_native_path(pathname,native_pathname);
	
		return MXJ_JNI_CALL(env,NewStringUTF)(env,native_pathname);
	}

	return NULL;
}

jobjectArray JNICALL mxj_get_search_path_forcontext(JNIEnv *env,jclass clazz)
{
	static method m = NULL;
	char maxpath[MAX_PATH_CHARS];
	char nativepath[MAX_PATH_CHARS];
	jobjectArray arr = NULL;
	long pathcount = 0;
	short *patharray = NULL;
	jstring *tmp = NULL;
	jstring js;
	int count;
	int i;
	
	if (!m) m = (method)gensym("__path_getsearchpath_forcontext__")->s_thing;
	
	if (m) {
		m(&pathcount, &patharray); // get an array of shorts
		if (patharray && pathcount) {
			MXJ_JNI_CALL(env, PushLocalFrame)(env, pathcount+1);
			checkException(env);
			
			tmp = (jstring *)sysmem_newptr(sizeof(jstring) * pathcount);
			if (tmp) {
				count = 0;
				for (i = 0; i < pathcount; i++) {
					path_topathname(patharray[i], "", maxpath);
					max_path_to_native_path(maxpath, nativepath);
					js = MXJ_JNI_CALL(env, NewStringUTF)(env, nativepath);
					checkException(env);
					tmp[count++] = js;
				}
				
				if (count) {
					arr = MXJ_JNI_CALL(env,NewObjectArray)(env, count, g_stringClass, NULL);
					checkException(env);
					for (i = 0; i < count; i++) {
						MXJ_JNI_CALL(env,SetObjectArrayElement)(env, arr, i, tmp[i]);
						checkException(env);
					}
				}
				
				sysmem_freeptr(tmp);
			}
			
			// we might want to cache this
			sysmem_freeptr(patharray);
		}
		
		MXJ_JNI_CALL(env, PopLocalFrame)(env, NULL);
		checkException(env);
	}
	
	return arr;
}

jobjectArray JNICALL mxj_get_search_path(JNIEnv *env,jclass clazz)
{
	short vol;
	char maxpath[MAX_PATH_CHARS];
	char nativepath[MAX_PATH_CHARS];
	//this is the maximumnumber of search paths that we will report -- 512
	jstring tmp[512];
	jstring js;
	int count;
	int i;
	jobjectArray arr;
	t_pathlink *searchpath = *((t_pathlink **)gensym("__pathlist_search__")->s_thing);
	
	MXJ_JNI_CALL(env,PushLocalFrame)(env,513);
	checkException(env);
	
	vol = 0;
	count = 0;
	while (!path_getnext(searchpath,&vol)) {
		if (count > 511)
			break;
		path_topathname(vol,"", maxpath);
		max_path_to_native_path(maxpath,nativepath);
		js = MXJ_JNI_CALL(env,NewStringUTF)(env,nativepath);
		checkException(env);
		tmp[count++] = js;
	}
	
	arr = MXJ_JNI_CALL(env,NewObjectArray)(env, count, g_stringClass, NULL);
	checkException(env);
	
	for (i = 0; i < count; i++) {
		MXJ_JNI_CALL(env,SetObjectArrayElement)(env, arr, i, tmp[i]);
		checkException(env);
	}
	
	
	MXJ_JNI_CALL(env,PopLocalFrame)(env,NULL);
	checkException(env);

	return arr;
}

jstring JNICALL mxj_nameconform(JNIEnv *env, jclass clazz,jstring jpath, jint style, jint type)
{
	short err;
	char *path;
	char pathout[MAX_PATH_CHARS];
	
	if (jpath) {
		path = (char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env,jpath,NULL);
		err = path_nameconform(path,pathout,style,type);
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,jpath,path);

		if (err) {
			error("(mxj) name conform error %d",err);
			return NULL;
		}
		
		return MXJ_JNI_CALL(env,NewStringUTF)(env,pathout);
	}
	
	return NULL;
}

jstring JNICALL mxj_maxpath_to_native_path(JNIEnv *env,jclass clazz, jstring max_path)
{
	short err;
	char *js;
	char pathin[MAX_PATH_CHARS];
	char pathout[MAX_PATH_CHARS];
	jstring jstr;
	
	if (max_path) {
		js = (char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env,max_path,NULL);
		strcpy(pathin,js);
		err = max_path_to_native_path(pathin,pathout);
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,max_path,js);
		
		if (err) {
			error("(mxj) error in max_path_to_native_path");
			return NULL;
		}
		
		jstr = MXJ_JNI_CALL(env,NewStringUTF)(env,pathout);
		
		return jstr;
	}
	
	return NULL;
}

jstring JNICALL mxj_get_preferences_path(JNIEnv *env, jclass clazz)
{
	short prefpath = 0;
	jstring jstr = NULL;

	if (!preferences_path("", false, &prefpath)) {
		char pathin[MAX_PATH_CHARS];
		char pathout[MAX_PATH_CHARS];

		if (!path_topathname(prefpath, "", pathin)) {
			if (!max_path_to_native_path(pathin,pathout)) {
				jstr = MXJ_JNI_CALL(env,NewStringUTF)(env,pathout);
			}
		}
	}

	if (!jstr) {
		error("(mxj) error in mxj_get_preferences_path");
	}

	return jstr;
}

#ifdef MAC_VERSION
extern int g_in_java_modal_dialog;
#endif
void JNICALL mxj_next_window_is_modal(JNIEnv *env,jclass clazz)
{
#ifdef MAC_VERSION
	g_in_java_modal_dialog = true;
#endif
}

/*
 * Return the Max version number.
 */
jshort JNICALL mxj_get_max_version(JNIEnv *env, jclass cls)
{
	return (jshort)maxversion();
}

/*
 * Return true if it's the runtime.
 */
jboolean JNICALL mxj_get_is_runtime(JNIEnv *env, jclass cls)
{
	long is_runtime = object_attr_getchar(gensym("max")->s_thing, gensym("isruntime"));
	
	if (is_runtime)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

/*
 * force load the object for things like jitter
 */
void JNICALL mxj_system_loadobject(JNIEnv *env, jclass cls, jbyteArray bytes)
{
	t_object *x;
	jint length = MXJ_JNI_CALL(env,GetArrayLength)(env, bytes);
	char *message;
	
	message = mxj_getbytes(length+1);

	if (message == 0) {
		JNU_ThrowByName(env, "java/lang/OutOfMemoryError", 0);
		return;
	}
	
	MXJ_JNI_CALL(env,GetByteArrayRegion)(env, bytes, 0, length, (jbyte *)message);
	message[length] = 0;
	
	x = newinstance(gensym(message),0,0);
	if (x)
		freeobject((t_object *)x);
	
	mxj_freebytes(message, length+1);
}


/*
 * Call error on the message using defer_low.
 */
void JNICALL mxj_system_error(JNIEnv *env, jclass cls, jbyteArray bytes)
{
	say(object_error, env, cls, bytes);
}


/*
 * Call ouchstring on the message using defer_low.
 */
void JNICALL mxj_system_ouch(JNIEnv *env, jclass cls, jbyteArray bytes)
{
	say(object_error_obtrusive, env, cls, bytes);
}

/*
 * Call post on the message using defer_low.
 */
void JNICALL mxj_system_post(JNIEnv *env, jclass cls, jbyteArray bytes)
{
	say(object_post, env, cls, bytes);
}

void JNICALL mxj_register_accelerator(JNIEnv* env, jclass clazz,jchar key)
{
	// this is no longer necessary -- happens automagically with Cocoa
}

void JNICALL mxj_unregister_accelerator(JNIEnv* env, jobject clazz,jchar key)
{
	// ddz this is no longer necessary -- happens automagically with Cocoa
}

jboolean JNICALL mxj_send_message_to_bound_object(JNIEnv *env, jclass clz, jstring name, jstring message, jobjectArray argv)
{
	char *msg;
	char *symname;
	short ac;
	t_atom *av, rv;
	t_symbol *s;
	t_max_err res;
	
	if (name == NULL || message == NULL)
		return JNI_FALSE;
	
	msg 	= (char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env,message,NULL);
	symname = (char *)MXJ_JNI_CALL(env,GetStringUTFChars)(env,name,NULL);
	
	s = gensym(symname);

	if (s->s_thing) {
		//convert java Atom[] to t_atom *
		if (argv != NULL) {
			av = newArgv(env,argv, &ac);
			res = object_method_typed((void *)s->s_thing, gensym(msg), ac, av, &rv);
			mxj_freebytes(av,ac * sizeof(t_atom));
		}
		else
			res = object_method_typed((void *)s->s_thing, gensym(msg), 0, NULL, &rv);
		
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,name,symname);
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,message,msg);
		
		if (res == MAX_ERR_NONE)
			return JNI_TRUE;

		return JNI_FALSE;
	}
	else {
		post("(mxj) MaxSystem.sendMessageToBoundObject: Nothing bound to symbol %s. Value of s_thing is 0.",symname);
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,name,symname);
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,message,msg);
		return JNI_FALSE;
	}
}

jdouble JNICALL mxj_systimer_gettime(JNIEnv *env, jclass clz)
{
	return (jdouble)systimer_gettime();
}

void mxj_max_system_register_natives(JNIEnv *env,jclass clazz)
{
	int count=0;
	JNINativeMethod methods[] =
	{
   		{ "doPost", "([B)V", mxj_system_post },
		{ "doLoadObject", "([B)V", mxj_system_loadobject },
		{ "doError", "([B)V", mxj_system_error },
   		{ "doOuch", "([B)V", mxj_system_ouch },
		{ "defer", "(Lcom/cycling74/max/Executable;)V", mxj_defer },
   		{ "deferLow", "(Lcom/cycling74/max/Executable;)V", mxj_defer_low },
   		{ "deferMedium", "(Lcom/cycling74/max/Executable;)V", mxj_defer_medium },
   		{ "deferFront", "(Lcom/cycling74/max/Executable;)V", mxj_defer_front },
   		{ "schedule", "(Lcom/cycling74/max/Executable;D)V", mxj_schedule },
   		{ "scheduleDelay", "(Lcom/cycling74/max/Executable;D)V", mxj_schedule_delay },
   		{ "scheduleDefer", "(Lcom/cycling74/max/Executable;D)V", mxj_schedule_defer },
   		{ "inMaxThread", "()Z", mxj_in_max_thread },
   		{ "inTimerThread", "()Z", mxj_in_timer_thread },
   		{ "inMainThread", "()Z", mxj_in_main_thread },
   		{ "hideCursor","()V",mxj_hide_cursor },
   		{ "showCursor","()V",mxj_show_cursor },
   		{ "openDialog","(Ljava/lang/String;)Ljava/lang/String;",mxj_open_dialog },
   		{ "saveAsDialog","(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",mxj_saveas_dialog },
   		{ "locateFile","(Ljava/lang/String;)Ljava/lang/String;",mxj_locate_file },
		{ "nameConform","(Ljava/lang/String;II)Ljava/lang/String;",mxj_nameconform},
		{ "doGetIsRuntime","()Z", mxj_get_is_runtime },
		{ "doGetMaxVersion","()S", mxj_get_max_version },
		{ "nextWindowIsModal","()V",mxj_next_window_is_modal},
		{ "maxPathToNativePath","(Ljava/lang/String;)Ljava/lang/String;",mxj_maxpath_to_native_path},
		{ "_register_command_accelerator","(C)V", mxj_register_accelerator },
   	  	{ "_unregister_command_accelerator","(C)V", mxj_unregister_accelerator },
   	  	{ "sendMessageToBoundObject","(Ljava/lang/String;Ljava/lang/String;[Lcom/cycling74/max/Atom;)Z", mxj_send_message_to_bound_object },
		{ "sysTimerGetTime","()D",mxj_systimer_gettime },
		{ "getSearchPathForContext","()[Ljava/lang/String;",mxj_get_search_path_forcontext },
		{ "getDefaultPath","()Ljava/lang/String;",mxj_get_default_path },
		{ "getSearchPath","()[Ljava/lang/String;",mxj_get_search_path },
		{ "getPreferencesPath","()Ljava/lang/String;",mxj_get_preferences_path },
		{ NULL, NULL, NULL }
	};
	
	while (methods[count].name) count++;
	
	MXJ_JNI_CALL(env,RegisterNatives)(env,clazz,methods,count);
	checkException(env);
}
