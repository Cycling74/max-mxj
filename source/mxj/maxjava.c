/*
 * maxjava.c - Write Max objects in Java.
 *
 * Author: Herb Jellinek/Topher LaFata
 */

#ifdef MAC_VERSION
#include <JavaVM/jni.h>        // Java Native Interface definitions
#include <JavaVM/jni_md.h>
#else
#include "jni.h"
#include "jni_md.h"
#endif

#include "buffer.h"
#include "IVirtualMachineAPI.h"

#include "ext_byteorder.h"
#include "ext_sysmem.h"
#include "ext_proto.h"
#include "dbg.h"
#include "package.h"
#include "Atom.h"
#include "mjglobals.h"
#include "maxjava.h"
#include "copyprot.h"
#include "classes.h"
#include "ExceptionUtils.h"
#include "callbacks.h"
#include "clock_callbacks.h"
#include "mxj_qelem.h"
#include "mxj_patcher.h"
#include "mxj_box.h"
#include "mxj_wind.h"
#include "mxj_max_system.h"
#include "classpath.h"
#include "threadenv.h"
#include "mxj_utils.h"
#include "mxj_symbols.h"
#include "mxj_msp_buffer.h"
#include "mxj_props.h"
#include "ext_obex.h"
#include "ext_packages.h"

#ifdef MAC_VERSION	// need for awt init

#include "ext_systhread.h"

typedef struct __CFRunLoop * CFRunLoopRef;

//ISO C requires named arg before elipsis
typedef void *(*t_mp)(void *arg,...); 
//method that takes no arguments.
//needed to get around the stupid elipsis crap
typedef void *(*t_mpV)(void); 
	
CFBundleRef c_bundle;
CFRunLoopRef rl; 		
t_mp PMO_GetCFRunLoopFromEventLoop;
t_mpV PMO_GetCurrentEventLoop;
t_mp PMO_CFRunLoopRunInMode;
t_mpV PMO_CFRunLoopRun;
t_mp PMO_CFRunLoopStop;
t_mp PMO_CFRunLoopSourceCreate;
t_mp PMO_CFRunLoopAddSource;

static t_systhread s_awt_init_thread;

void awt_init_func(void);

// Carbon/Swing modal dialog fix
int g_in_java_modal_dialog = false;

#endif // MAC_VERSION

#ifdef WIN_VERSION
#include "mxj_win.h"

extern InvocationFunctions g_ifn;
#endif

// Debugging
#define DEBUG FALSE		// Set this to TRUE to turn on debug messages

#define GET_INLET_COUNT_MNAME "getNumInlets"
#define GET_INLET_COUNT_SIG "()I"
#define GET_INLET_TYPE_MNAME "getInletType"
#define GET_INLET_TYPE_SIG "(I)I"
#define GET_INLET_ASSIST_MNAME "getInletAssist"
#define GET_INLET_ASSIST_SIG "(I)Ljava/lang/String;"
#define GET_OUTLET_COUNT_MNAME "getNumOutlets"
#define GET_OUTLET_COUNT_SIG "()I"
#define GET_OUTLET_TYPE_MNAME "getOutletType"
#define GET_OUTLET_TYPE_SIG "(I)I"
#define GET_OUTLET_ASSIST_MNAME "getOutletAssist"
#define GET_OUTLET_ASSIST_SIG "(I)Ljava/lang/String;"
#define INIT_MXJ_MESSAGE_TABLE_MNAME  "_init_mxj_message_table"
#define INIT_MXJ_MESSAGE_TABLE_SIG    "()V"
#define INIT_MXJ_ATTR_TABLE_MNAME  "_init_mxj_attr_table"
#define INIT_MXJ_ATTR_TABLE_SIG    "()V"
#define INLET_BANG_NAME "bang"
#define INLET_BANG_SIG "()V"
#define INLET_MNAME "inlet"
#define INLET_INT_SIG "(I)V"
#define INLET_FLOAT_SIG "(F)V"
#define INLET_MESSAGE_SIG "(Ljava/lang/String;[L"MAX_ATOM_CLASSNAME";)V"
#define INLET_ATOMLIST_SIG "([L"MAX_ATOM_CLASSNAME";)V"
#define ANYTHING_MNAME "anything"
#define ANYTHING_SIG   "(Ljava/lang/String;[L"MAX_ATOM_CLASSNAME";)V"
// void mxjObjectWasDeleted() - notification that the mxj C counterpart to the Java object
// was deleted/freed.
#define MXJDELETED_MNAME "mxjObjectWasDeleted"
#define MXJDELETED_SIG "()V"

#ifdef MXJ_MSP
	#define MXJ_DSP_NAME "dsp"
	#define MXJ_DSP_SIG "([Lcom/cycling74/msp/MSPSignal;[Lcom/cycling74/msp/MSPSignal;)Ljava/lang/reflect/Method;"	
	#define MXJ_DSPSTATE_NAME "dspstate"
	#define MXJ_DSPSTATE_SIG "(Z)V"	
	t_symbol *ps_dsp,*ps_stop,*ps_start;
#endif

#define O_TYPE_INT		1
#define O_TYPE_FLOAT	2
#define O_TYPE_LIST		4
#define O_TYPE_MESS		8
#define O_TYPE_BANG		16

#ifdef MXJ_MSP
	#define I_TYPE_SIGNAL 32
	#define O_TYPE_SIGNAL 32
#endif

#ifdef MXJ_MSP
//benchmarking
extern double systimer_gettime();
static double _t_1, _t_2;
#define BENCHMARK_BEGIN _t_1 = systimer_gettime()
#define BENCHMARK_END   _t_2 = systimer_gettime()
#define BENCHMARK_RESULT (_t_2 - _t_1) * (1000.)

// if we change mxj~ to pass 64bit audio in and out of java
// then we want to change the following to be a double - jkc
typedef float t_mxj_sample;

#endif

// Java world

JavaVM *g_jvm = NULL;			// the single Java virtual machine we will create
char g_java_jvm_version[32];	// on OS X this can be set via max.java.config.txt
t_symbol *ps_global_jvm;		// if it is already around we use this symbol to bind to it
t_symbol *ps_global_props;

char *g_mxj_classpath = NULL;

#define JVM_CONFIG    "max.java.config.txt"
#define PROP_CP_DIR  "max.system.class.dir"
#define PROP_JAR_DIR "max.system.jar.dir"
#define PROP_DYNAMIC_CLASS_DIR  "max.dynamic.class.dir"
#define PROP_DYNAMIC_JAR_DIR    "max.dynamic.jar.dir"
#define PROP_JVM_OPT "max.jvm.option"
#define PROP_DEBUG   "mxj.debug"
#define PROP_CLASSES_FROM_DISK "mxj.classloader.fromdisk" //leave this in so we don't complain even though it is never used anymore
#define PROP_MXJ_MSP_MODE "mxj.msp.mode"
//this is just used on OS X tiger. Person can specify if they want to use the 1.5 JVM
//inside max
#define PROP_JAVA_JVM_VERSION "max.java.jvm.version"

#define MXJ_MSP_MODE_COMPATIBLE		1
#define MXJ_MSP_MODE_HIPERFORMANCE	2

/*
 the HIPERFORMANCE mode optimizations rely on knowledge of the jarray header size,
 which will be a struct containing (in the case of a SUN VM)
 
 mark -> native word size
 reference -> java ref size
 count -> 32 bit integer
 
 generally all objects have to be 8 byte aligned on all platforms
 
 so for 32 bit systems we have a 12 bytes header (a java reference will be a 32 bit pointer)
 
 on 64 systems this is different - the size of a java reference can vary - it depends on the
 value of the option: "UseCompressedOops" - if it is switched on it will use 4 byte references
 to 8 byte aligned memory instead of full 8 byte references (which would be normal for 64bit),
 which means some kind of hybrid addressing, it is an optimization to save memory and
 should be faster - see: https://wikis.oracle.com/display/HotSpotInternals/CompressedOops
 
 this option is said to be on by default since Java 6 update 33 (released June 2012) - so with
 this option set to on, we will have a 16 bytes header, with this option off we will have
 a 24 bytes header
 
 we decided to assume that this option is enabled for HIPERFORMANCE mode, so this mode requires
 a Java VM > 6u33
 
 as an alternative the use of DirectByteBuffer might be worth exploring, but this would require
 a rewrite of the MaxSignal Java object
 */

typedef void* t_mark;
#ifdef C74_X64
typedef t_int32 t_Oop;
#else
typedef void* t_Oop;
#endif
typedef struct java_array_ref {
	t_mark mark;
	t_Oop clz;
	t_uint32 length;
} t_java_array_ref;

// Max/Java
static int g_triedFrameworkInit = FALSE;
static int g_triedGlobalsInit = FALSE;

static t_class *s_maxjava_class;

static t_mxj_proplist *props = NULL;

/*
 * Prototypes for the methods that are defined or used below
 */

// initializers
void initFrameworkClass(JNIEnv *env);
void initGlobals(JNIEnv *env);
void init_mxj_jitter(JNIEnv *env);
#ifdef MAC_VERSION
CFBundleRef getMachOLibrary(CFStringRef bundleName);
short init_awt();
#endif

/**
 the Java class constructor used in maxjava_new
 
 @param 	env			our Java environment
 @param 	className	the name of the Java class to instantiate
 @param		x			the maxjava object
 @param 	argc		The count of arguments given to mxj/mxj~
 @param 	argv		Array of t_atoms; the arguments given to mxj/mxj~
*/
t_mxj_err construct(JNIEnv *env, char *className, t_maxjava *x, short argc,t_atom* argv);

// helper func for the constructor
t_mxj_err constructor_excep_helper(JNIEnv *env);

// Max object methods

/*
 * We received bang on an inlet.  Call void bang() on java object
 *
 */
void maxjava_bang(t_maxjava *x);

/*
 * We received an int on an inlet.
 */
void maxjava_int(t_maxjava *x, long n);

/*
 * We received a float on an inlet.
 */
void maxjava_float(t_maxjava *x, double n);

/*
 * We received some other kind of message on an inlet.
 */
void maxjava_anything(t_maxjava *x, t_symbol *msg, short argc, t_atom *argv);

/*
 * Provide assistance as to the nature of the inlet or outlet the user is pointing at.
 */
void maxjava_assist(t_maxjava *x, void *b, long m, long a, char *s);

/*
 * Handle quickref.
 */
void maxjava_quickref(t_maxjava *x, long *numitems, t_symbol **items);

/*
 * Handle patcher saving.
 */
void mxj_save2(t_maxjava *x, void *z);

/*
 * Deliver loadbang to Java.
 */
void mxj_loadbang(t_maxjava *x);

/*
 * Deliver double click on box to Java.
 */
void mxj_dblclick(t_maxjava *x);

/*
 * Tell the Max standalone builder about our dependencies.
 */
void mxj_fileusage(t_maxjava *x, void *w);

/*
 * Create and return a new mxj instance.
 */
void *maxjava_new(t_symbol *s, short argc, t_atom *argv);

/*
 * Free the mxj/mxj~object.
 */
void maxjava_free(t_maxjava *x);

/*
 * Provide the classname, used to find help patcher.
 */
void maxjava_objectfilename(t_maxjava *x, char *s);

/**
 call a java constructor trying to coerce argument list to match the types
 given in sig (see t_mxj_method->jp_types) - used via construct()
 
 @param 	x		the maxjava object
 @param 	mid		constructor ID
 @param		sig		java siganture
 @param 	argc	The count of arguments in argv
 @param 	argv	Array of t_atoms; the method arguments
 */
t_mxj_err call_constructor_with_coercion(t_maxjava *x, jmethodID cid, t_symbol *sig, short argc, t_atom *argv);

// Java-related

/*
 * Allocate a JVM, which we will deallocate when we exit.
 * Sets various runtime options and sticks max.jar, example.jar, the optional tests.jar,
 * and the CLASS_PATH directory itself on the classpath.
 * Returns the JNIEnv * for this thread, or NULL if it failed to start Java.
 */
JNIEnv *jvm_new(long *exists);

/*
 * Max is exiting.  If we've allocated a JVM, destroy it.
 */
void jvm_release(void);

/*
 * Read JVM options from file defined in JVM_CONFIG.
 */
short mxj_get_jvmopts(JavaVMOption* options, int *num_options, int max_opts);

// some helper functions to construct and post the classpath in get_jvm_opts
void cp_add_system_jar_dir(char *native_dirname, short path, t_string *classpath);
void cp_add_dynamic_class_dir(char* native_dirname);
void cp_post_system_classpath(char *sys_classpath);

// Class com.cycling74.max.MaxObject: the class from which all mxj-compatible classes must derive
static jclass g_frameworkClass;
static jclass g_max_runtime_exception_clz;
// Method to call to notify a MaxObject instance that it's mxj is deleted
static jmethodID g_mxjObjectWasDeleted_MID;

/*
 * Set the Java instance's mPeer field to point at the t_maxjava object.
 */
void setPeer(JNIEnv *env, t_maxjava *x);

/*
 * Set the Java instance's mPeer field to be NULL.  We call this from maxjava_free,
 * and check for the NULL peer in outlet methods.
 */
void unsetPeer(JNIEnv *env, t_maxjava *x);

/*
 * Set the Java instance's mName field to point to the class loaded.
 */
void setName(JNIEnv *env, t_maxjava *x, char* name);

/*
 * Generate our inlets
 */
void mxj_make_inlets(JNIEnv *env, t_maxjava *x);

/*
 * Generate our outlets
 */
void mxj_make_outlets(JNIEnv *env, t_maxjava *x);

/*
 * Notify the MaxObject that the mxj object has been deleted.
 */
void mxjObjectWasDeleted(JNIEnv *env, t_maxjava *x);

/*
 * Notify the classloader of all our collected classpaths.
 */
void mxj_inform_classloader();

/*
 * Get a prop value - default is the default value you would
 * like to be passed back in the case where prop with prop_id does not exist.
 */
void* mxj_get_prop_val(int prop_id, void* default_val);

//@ args handling
t_at_exec_unit *at_exec_unit_new(t_symbol *msg);
void at_exec_unit_push(t_at_exec_unit *x, t_atom *a);
void at_exec_unit_free(t_at_exec_unit* x);
void at_exec_unit_print(t_at_exec_unit *x);
t_at_exec_unit* get_exec_unit(int *counter,short argc, t_atom *argv);
void mxj_add_at_exec_unit(t_maxjava *x,t_at_exec_unit *aeu);
void parse_out_at_args(t_maxjava * x, short *argc,t_atom *argv);
void mxj_at_exec_units_exec(t_maxjava *x, int low_priority);

//attr messages..respond to get foo 
void maxjava_handle_get(t_maxjava *x, t_symbol *msg, short argc, t_atom *argv); 
int maxjava_do_attr_get(t_maxjava *x, t_symbol *attr_name);

void* g_sched_peer   = NULL; //max peer obj during construction in high-priority thread
void* g_main_peer    = NULL; //max peer obj during construction in low priority thread

#ifdef MXJ_MSP

void mxj_dsp64(t_maxjava *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void mxj_dsp_add64(t_maxjava *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags, short mode);
void mxj_perform_compatible64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void mxj_perform_compatible_benchmark64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void mxj_perform_compatible_exception_check64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

void mxj_perform_hiperformance64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void mxj_perform_hiperformance_benchmark64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void mxj_perform_hiperformance_exception_check64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

void mxj_dspstate(t_maxjava *x, long n);
void mxj_benchmark(t_maxjava *x, long way, long interval);
void mxj_exception_check(t_maxjava *x,int way);
void mxj_dsp_halt(t_maxjava *x);
void mxj_bounce_dac();

jboolean mxj_check_z_no_inplace(JNIEnv *env, t_maxjava *x);

#endif // MXJ_MSP

void mxj_string_appendtoclasspath(t_string *classpath, char *jarpath);

//--------------------------------------------------------------------------
#ifdef MAC_VERSION
void ext_main(void *r) {
#else
void ext_main(void *r) {
#endif
	t_class *c;

	c = class_new(MAX_CLASSNAME, (method)maxjava_new, (method)maxjava_free, sizeof(t_maxjava), (method)NULL, A_GIMME, 0);
	
#ifdef MXJ_MSP
	class_dspinit(c);
#endif
	class_addmethod(c, (method)maxjava_int,						"int", A_LONG, 0);
	class_addmethod(c, (method)maxjava_float,					"float", A_FLOAT, 0);
	class_addmethod(c, (method)maxjava_bang,					"bang",	0);

	class_addmethod(c, (method)maxjava_anything,				"list", A_GIMME, 0);
	class_addmethod(c, (method)maxjava_anything,				"anything", A_GIMME, 0);
	class_addmethod(c, (method)maxjava_anything,				"viewsource", A_GIMME, 0);
	class_addmethod(c, (method)maxjava_anything,				"zap", A_GIMME, 0);
	
	class_addmethod(c, (method)maxjava_quickref,				"quickref",A_CANT,0);
	class_addmethod(c, (method)maxjava_assist,					"assist", A_CANT, 0);
	class_addmethod(c, (method)maxjava_handle_get,				"get", A_GIMME, 0);
	class_addmethod(c, (method)maxjava_objectfilename,			"objectfilename", A_CANT, 0);
	class_addmethod(c, (method)mxj_save2,						"save2", A_CANT, 0);
	class_addmethod(c, (method)mxj_loadbang,					"loadbang",A_CANT,0);
	class_addmethod(c, (method)mxj_dblclick,					"dblclick",A_CANT,0);
	class_addmethod(c, (method)mxj_fileusage,					"fileusage", A_CANT, 0);
#ifdef MXJ_MSP
	class_addmethod(c, (method)mxj_dsp64,						"dsp64", A_CANT, 0);
	class_addmethod(c, (method)mxj_dspstate,					"dspstate", A_CANT, 0);
	class_addmethod(c, (method)mxj_benchmark,					"benchmark",A_DEFLONG,A_DEFLONG,0);
	class_addmethod(c, (method)mxj_exception_check,				"exceptioncheck",A_DEFLONG,0);
#endif
	
	class_register(CLASS_BOX, c);
	s_maxjava_class = c;
	
	quittask_install((method)jvm_release, NULL);
		
	//better to do this using oblist message in init folder -jkc
	finder_addclass("System", MAX_CLASSNAME);  //   this in the Max Finder. 

	//stuff that needs to happen before the JVM is instantiated
	ps_global_props = gensym("_#MAX_JAVA_PROPS#_");
	if (ps_global_props->s_thing)
		props = (t_mxj_proplist *)ps_global_props->s_thing;
	else
	{
		props = mxj_proplist_new(32);
		ps_global_props->s_thing = (void*)props;
	}

	ps_global_jvm = gensym("_#MAX_JAVA_VM#_");
#ifdef MAC_VERSION 
return 0;
#endif
}

void mxj_save2(t_maxjava *x, void *z)
{
	JNIEnv *env;
	static jmethodID mid = 0;
	THREADENV(env);

	if (!mid)
	{
		mid = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls,"save", "()V");
		checkException(env);
	}
	 
	x->binbuf = z;
	MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance,mid);

	x->binbuf = NULL;
		
}


void *maxjava_new(t_symbol *s, short argc, t_atom *argv)
{
	t_maxjava *x;
	t_mxj_err ret;
    int i;
    char *maxObjClassName;
    char className[MAX_CLASSNAME_LENGTH];
    jmethodID init_mxj_method_table_mid; 
    jmethodID init_mxj_attr_table_mid;  
    JNIEnv *env = NULL;// The Java environment for this thread
	jboolean conforms = JNI_FALSE;// Is it a subclass of MaxObject?

		
	x = (t_maxjava *)object_alloc(s_maxjava_class);
	gensym("#X")->s_thing = (t_object*)x; 	//for use with the save/retore mechanism	
								// stored in patcher in the form: #X message atom1 atom2 ... atom254; 
								// e.g. write #X rgb 255 0 37; then need to have a message named rgb 
								// which will receive this message after instantiation
	
	maxObjClassName = s->s_name; //mxj or mxj~
	x->p_classname  = 0;
	
	if (argc == 0) {
		error("(%s) Must specify a class to load", maxObjClassName);
		return 0;
	}
	if (argv[0].a_type == A_LONG) {
		error("%ld is not a legal class name", argv[0].a_w.w_long);
		return 0;
	} else if (argv[0].a_type == A_FLOAT) {
		error("%f is not a legal class name", argv[0].a_w.w_float);
		return 0;
	} else if (argv[0].a_type == A_SYM) {
		long length = (long)strlen(argv[0].a_w.w_sym->s_name);
		if (length > MAX_CLASSNAME_LENGTH - 1) {
			error("Class name is too long (%d chars)", length);
			return 0;
		} else if (length == 0) {
			error("No class name supplied"); 
			return (0);
		}
	}
	strcpy(className, argv[0].a_w.w_sym->s_name);
	
	if (g_jvm == NULL) {
		long exists = 0;
		env = jvm_new(&exists);
		if (env == NULL) {
			error("Unable to create JVM");
			return 0; 
		}
#ifdef MAC_VERSION
		c_bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.Carbon"));
		PMO_GetCFRunLoopFromEventLoop  = (t_mp) CFBundleGetFunctionPointerForName(c_bundle, CFSTR("GetCFRunLoopFromEventLoop"));
		PMO_GetCurrentEventLoop = (t_mpV) CFBundleGetFunctionPointerForName(c_bundle, CFSTR("GetCurrentEventLoop"));
		PMO_CFRunLoopRunInMode = (t_mp) CFBundleGetFunctionPointerForName(c_bundle, CFSTR("CFRunLoopRunInMode"));
		PMO_CFRunLoopRun = (t_mpV) CFBundleGetFunctionPointerForName(c_bundle, CFSTR("CFRunLoopRun"));
		PMO_CFRunLoopStop = (t_mp) CFBundleGetFunctionPointerForName(c_bundle, CFSTR("CFRunLoopStop"));	
		
		rl = PMO_GetCFRunLoopFromEventLoop(PMO_GetCurrentEventLoop());
		if (!exists) { // only do this for the first one of mxj or mxj~ loaded -jkc
//			t_symbol *ps_sched_disablequeue = gensym("sched_disablequeue"); 
//			method sched_disablequeue = (method) ps_sched_disablequeue->s_thing; 
//			long oldval; 
//			init_awt();
//			// a very hacky way to make sure that awt is already initialized on the other thread
//			// this prevents a hang on OS X 10.9 Mavericks
//			systhread_sleep(2000);
//			// we disable queue servicing while running the run loop here 
//			if (sched_disablequeue)
//				oldval = (long) (*sched_disablequeue)((void*) 1); 
//			PMO_CFRunLoopRun();//enter CF runloop so cocoa can call back to us. Thread spawned in init_awt will break us out.
//			if (sched_disablequeue)
//				(*sched_disablequeue)((void*) oldval); 
		}
#endif	//MAC_VERSION
	} else {
		THREADENV(env);
	}

	initFrameworkClass(env);
 
    if (g_frameworkClass == NULL) {
    	error("Could not find framework class %s", MAX_FRAMEWORK_CLASS);
		return 0;
	}
	
	initGlobals(env);

	DBG(post("Class to load is '%s'", className));
	
	x->thispatcher = (t_patcher *)gensym("#P")->s_thing;		
	x->thisbox = (t_box*)gensym("#B")->s_thing;
	
	if (mxj_classloader_load_class(env, className, &x->cls) != MAX_ERR_NONE)
		return NULL;

    // check that it subclasses MAX_FRAMEWORK_CLASS
	conforms = MXJ_JNI_CALL(env,IsAssignableFrom)(env, x->cls, g_frameworkClass);
	if (conforms == JNI_TRUE) {
		jobject tmp;
				
		//copy 'command line' args so that we can save ourself in save method
		x->argc = (long)argc+2;
		x->args = (t_atom*) mxj_getbytes((x->argc) * sizeof(t_atom));
		A_SETSYM(x->args,gensym("#N"));//put mxj in here
		A_SETSYM(x->args+1,s);//put mxj in here
		for(i = 2; i < x->argc;i++)
			x->args[i] = argv[i-2];
			
		// construct the instance. we are trimming the class name from the arglist since
		//it is explicity set by us below. 
		if (argc == 1)
			argc = 0;
		else
			argc--;argv++;	
		
		// get the @ arguments. this has the side effect of changing argc.
		x->aeu_num  = 0;
		x->aeu_list = NULL;
		
		parse_out_at_args(x, &argc, argv);
		
		tmp = MXJ_JNI_CALL(env,AllocObject)(env,x->cls);
		if (checkException(env) || !tmp)
		{
			error("(%s) unable to alloc instance of %s", maxObjClassName,className);
			MXJ_JNI_CALL(env,DeleteLocalRef)(env,tmp);
			return 0;
		}
		
		x->javaInstance = tmp;
		// stick a pointer to this Max object (x) in the Java object (x->javaInstance)
		setPeer(env, x);
		
		// we need to fill out our method list before we call construct since the constructors end
		// up in the methodlist after calling init_mxj_message_table
		x->methodlist = mxj_methodlist_new();
				
		init_mxj_method_table_mid = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls,INIT_MXJ_MESSAGE_TABLE_MNAME, INIT_MXJ_MESSAGE_TABLE_SIG);
	    MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance,init_mxj_method_table_mid);
		checkException(env);
		
		/* this is so people can call back into native code in the constructor of their
		 MaxObject. For instance, this._p = this.getParentPatcher(). Objects should ever
		 only be created in one of the max threads since all java threads are deferred to
		 the main thread or in some cases the scheduler thread.
		 */
		if(systhread_ismainthread())
			g_main_peer  = x;
		else if(systhread_istimerthread())
			g_sched_peer = x;
		else//this should never happen
			error("(mxj) Constructing object from unknown thread!!!This is no good. How did it happen!");
		
		ret = construct(env, className, x, argc, argv);
	
		if(systhread_ismainthread())
			g_main_peer  = NULL;
		else if(systhread_istimerthread())
			g_sched_peer = NULL;
		
		if (ret == MXJ_ERR_NONE) {
			x->javaInstance = MXJ_JNI_CALL(env,NewGlobalRef)(env,tmp);
			MXJ_JNI_CALL(env,DeleteLocalRef)(env,tmp);
		} else {
			if (ret != MXJ_ERR_BAIL)
				error("(%s) unable to construct instance of %s", maxObjClassName,className);
			MXJ_JNI_CALL(env,DeleteLocalRef)(env,tmp);
			return 0;
		}
		
		//we need to set the peer again since it is zeroed out upon construction
		setPeer(env, x);
			
		x->p_classname = mxj_getbytes(strlen(className) + 1);
		strcpy(x->p_classname, className);

		x->inlet_bang_MID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, INLET_BANG_NAME, INLET_BANG_SIG);
		checkException(env);
		x->inlet_int_MID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, INLET_MNAME, INLET_INT_SIG);
		checkException(env);
		x->inlet_float_MID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, INLET_MNAME, INLET_FLOAT_SIG);
		checkException(env);
		x->inlet_anything_MID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, ANYTHING_MNAME, ANYTHING_SIG);
		checkException(env);
		x->getInletAssist_MID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, GET_INLET_ASSIST_MNAME,GET_INLET_ASSIST_SIG);
		checkException(env);  
		x->getOutletAssist_MID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, GET_OUTLET_ASSIST_MNAME,GET_OUTLET_ASSIST_SIG);
		checkException(env);

    	mxj_make_inlets(env,x);
		mxj_make_outlets(env,x);

	#ifdef MXJ_MSP	
		x->dsp_MID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, MXJ_DSP_NAME, MXJ_DSP_SIG);
		checkException(env);
		x->dspstate_MID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, MXJ_DSPSTATE_NAME, MXJ_DSPSTATE_SIG);			
  		checkException(env);  		
   		x->aenv                 = 0;
   		x->benchmark_perform    = 0;
   		x->perf_exception_check = 0;
    	x->dsp_call_cnt         = 0;				
 		if (mxj_check_z_no_inplace(env,x))
 				x->p_ob.z_misc = Z_NO_INPLACE;	
    #endif// MXJ_MSP
    
		x->binbuf     = NULL;
	
		x->attrlist   = 0;
		x->attr_num   = 0;
		init_mxj_attr_table_mid = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, INIT_MXJ_ATTR_TABLE_MNAME, INIT_MXJ_ATTR_TABLE_SIG);
	    MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance, init_mxj_attr_table_mid);
		checkException(env);
		//do @ stuff
		//high priority attributes?  use the line below instead of the one above:
		//mxj_at_exec_units_exec(x,1);
		mxj_at_exec_units_exec(x,0);		
		
		return x;	// return a reference to the object instance
	} else {
		error("(%s) Class %s is not a subclass of %s",maxObjClassName, className, MAX_FRAMEWORK_CLASS);
		return NULL;
	}
}


void initFrameworkClass(JNIEnv *env)
{
    // check for existence of framework class - do this once
    if (!g_triedFrameworkInit) {		
	    g_frameworkClass = getClassByName(env, MAX_FRAMEWORK_CLASS);
    	g_triedFrameworkInit = TRUE;
	}
}

/*
 * Initialize the various global objects.
 */
void initGlobals(JNIEnv *env)
{
	if (!g_triedGlobalsInit) {
	
		MXJ_JNI_CALL(env,PushLocalFrame)(env,32);
		
		// get a ref to the Atom class
		g_atomClass = getClassByName(env, MAX_ATOM_CLASSNAME);
	   	checkException(env);
	 	g_stringClass = getClassByName(env,"java/lang/String");
	 	checkException(env);
	 	g_ObjectClass = getClassByName(env,"java/lang/Object");
	 	checkException(env);
	 	g_max_runtime_exception_clz = getClassByName(env,"com/cycling74/max/MaxRuntimeException");
		checkException(env);
#ifdef MXJ_MSP                                
		g_msp_sig_clz = getClassByName(env,"com/cycling74/msp/MSPSignal");
		checkException(env);
		g_msp_sig_const = MXJ_JNI_CALL(env,GetMethodID)(env, g_msp_sig_clz,"<init>", "([FDIS)V");		
		checkException(env);
		EMPTY_MSPSIG_ARRAY = MXJ_JNI_CALL(env,NewObjectArray)(env,0,g_msp_sig_clz,NULL);
		EMPTY_MSPSIG_ARRAY =  MXJ_JNI_CALL(env,NewGlobalRef)(env,EMPTY_MSPSIG_ARRAY);			
#endif	 	
	 	
	    // init the static factory method IDs
	    g_newIntAtom_MID = MXJ_JNI_CALL(env,GetStaticMethodID)(env, g_atomClass,NEW_INT_ATOM_MNAME, NEW_INT_ATOM_SIG);
		checkException(env);
	    g_newFloatAtom_MID = MXJ_JNI_CALL(env,GetStaticMethodID)(env, g_atomClass, NEW_FLOAT_ATOM_MNAME, NEW_FLOAT_ATOM_SIG);
		checkException(env);
	    g_newStringAtom_MID = MXJ_JNI_CALL(env,GetStaticMethodID)(env, g_atomClass, NEW_STRING_ATOM_MNAME, NEW_STRING_ATOM_SIG);
		checkException(env);	    											  
		g_getIntValue_MID = MXJ_JNI_CALL(env,GetMethodID)(env, g_atomClass,GET_INT_VALUE_MNAME, GET_INT_VALUE_SIG);
		checkException(env);
		g_getFloatValue_MID = MXJ_JNI_CALL(env,GetMethodID)(env, g_atomClass, GET_FLOAT_VALUE_MNAME, GET_FLOAT_VALUE_SIG);
		checkException(env);
		g_getStringValue_MID = MXJ_JNI_CALL(env,GetMethodID)(env, g_atomClass, GET_STRING_VALUE_MNAME, GET_STRING_VALUE_SIG);
		checkException(env);
	    g_getAtomType_MID = MXJ_JNI_CALL(env,GetMethodID)(env, g_atomClass, GET_ATOM_TYPE_MNAME, GET_ATOM_TYPE_SIG);
		checkException(env);
		g_mxjObjectWasDeleted_MID = MXJ_JNI_CALL(env,GetMethodID)(env, g_frameworkClass, MXJDELETED_MNAME, MXJDELETED_SIG);
		checkException(env);
		g_java_jvm_version[0] = '\0'; 
		
#ifdef MXJ_MSP
		ps_dsp 	 = gensym("dsp");
		ps_start = gensym("start");
		ps_stop  = gensym("stop");
#endif
    	
      	J_BOOL       = gensym("Z");
  		J_BYTE       = gensym("B");
  		J_CHAR       = gensym("C");
  		J_SHORT      = gensym("S");
  		J_INT        = gensym("I");
  		J_LONG       = gensym("J");
  		J_FLOAT      = gensym("F");
  		J_DOUBLE     = gensym("D");
  		J_STRING     = gensym("s");
  		
  		J_BOOL_ARRAY       = gensym("[Z");
  		J_BYTE_ARRAY       = gensym("[B");
  		J_CHAR_ARRAY       = gensym("[C");
  		J_SHORT_ARRAY      = gensym("[S");
  		J_INT_ARRAY        = gensym("[I");
  		J_LONG_ARRAY       = gensym("[J");
  		J_FLOAT_ARRAY      = gensym("[F");
  		J_DOUBLE_ARRAY     = gensym("[D");
  		J_STRING_ARRAY     = gensym("[s");
  		 	
  		MAX_LONG     = gensym("I");
  		MAX_FLOAT    = gensym("F");
  		MAX_SYM      = gensym("S");
  		SYM_TRUE     = gensym("true");
  		SYM_FALSE    = gensym("false");
  		ATTR_VIRTUAL = gensym("virtual");	
    
        init_max_object_callbacks(env);
		init_clock_callbacks(env);
		init_mxj_max_system(env);
		init_mxj_qelem(env);
		init_mxj_patcher(env);
		init_mxj_box(env);
		init_mxj_wind(env);	
		init_mxj_mspbuffer(env);
		init_mxj_jitter(env);
		
		g_triedGlobalsInit = TRUE;
		
		MXJ_JNI_CALL(env,PopLocalFrame)(env,NULL);
	}
}
	
void mxjObjectWasDeleted(JNIEnv *env, t_maxjava *x)
{
	MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance, g_mxjObjectWasDeleted_MID);
	checkException(env);
}

//
// Class loading and object construction
//
/* call appropriate constructor on our alloced object */
t_mxj_err construct(JNIEnv *env, char *className, t_maxjava *x, short argc, t_atom* argv)
{	
	t_mxj_method *m;
	char *p_types;
	int i,resolve_stat, meth_offset;
	
	p_types = mxj_getbytes(argc+2);	
	p_types[0] = '\0';	
	if (argc == 0) {
		p_types[0] = 'V';
		p_types[1] = '\0';
	
	} else {	// build param types of incoming message. Can only be one of 3 things in max rite now.
		for(i = 0; i < argc; i++) {
			switch (argv[i].a_type) {
				case A_LONG:
					p_types[i] = 'I';
					break;
				case A_FLOAT:
					p_types[i] = 'F';
					break;
				case A_SYM:
					p_types[i] = 's';
					break;
				default:
					break;
			}
		}
		
		p_types[i] = '\0';
	}
	
	m = mxj_methodlist_resolve(x->methodlist, "<init>", argc, p_types, &resolve_stat, &meth_offset);
   	mxj_freebytes(p_types, argc +2);

	if (m) {
		//Note: order of precedence is GIMME, ARRAY, EXACTPARAM_MATCH, NOEXACT_PARAMMATCH
		return call_constructor_with_coercion(x, m->mids[meth_offset], m->jp_types[meth_offset], argc, argv);
	} else {
		object_error((t_object *)x,"could not find constructor");
		return MXJ_ERR_GENERIC;
	}
}

//
// Inlet methods
//
#ifndef MXJ_MSP
	#define PROXY_GETINLET(x) proxy_getinlet((t_object *)x)
#endif
void maxjava_bang(t_maxjava *x)
{
	JNIEnv *env;	
	THREADENV(env);
	MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance, x->inlet_bang_MID);
	checkException(env);
}

void maxjava_int(t_maxjava *x, long n)
{
	JNIEnv *env;

	THREADENV(env);
	MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance, x->inlet_int_MID, n);
	checkException(env);
}

void maxjava_float(t_maxjava *x, double n)
{
	JNIEnv *env;
	THREADENV(env);
	MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance, x->inlet_float_MID, n);
	checkException(env);
}

void maxjava_objectfilename(t_maxjava *x, char *s)
{
	if (x->p_classname)
		strcpy(s,x->p_classname);
	else
		strcpy(s,"mxj");
}

void maxjava_quickref(t_maxjava *x, long *numitems, t_symbol **items)
{
	int i,p,q;
	int c_idx;
	t_mxj_attr *a;
	char str[128];
	JNIEnv *env;
	c_idx  = 0;
	
	THREADENV(env);
	
	if (*numitems != 0)
		return;
	
	items[c_idx++] = gensym("ATTRIBUTES");
	for(i = 0; i < x->attr_num; i++) {
		short ac;
		t_atom *av;
		char* aname;
		
		str[0] = '\0';
		a = x->attrlist[i];
		aname = a->name->s_name;
		ac = 0;
		av = 0;	
		
		if (a->j_f_type == J_INT) {
			a->getter((void *)env,x,a,&ac,&av);
			sprintf(str,"  %s [int] %ld",aname,(long)av[0].a_w.w_long);
		}
		else if (a->j_f_type == J_FLOAT) {
			a->getter((void *)env,x,a,&ac,&av);
			sprintf(str,"  %s [float] %f",aname,av[0].a_w.w_float);
		}
		else if (a->j_f_type == J_STRING) {
			a->getter((void *)env,x,a,&ac,&av);
			sprintf(str,"  %s [string] %s",aname,(av == 0)?"NULL":av[0].a_w.w_sym->s_name);
		}
		else if (a->j_f_type == J_LONG) {
			a->getter((void *)env,x,a,&ac,&av);
			sprintf(str,"  %s [long] %ld",aname,(long)av[0].a_w.w_long);
		}
		else if (a->j_f_type == J_DOUBLE) {
			a->getter((void *)env,x,a,&ac,&av);
			sprintf(str,"  %s [double] %f",aname,av[0].a_w.w_float);
		}
		else if (a->j_f_type == J_BOOL) {
			a->getter((void *)env,x,a,&ac,&av);
			sprintf(str,"  %s [bool] %s",aname,(av[0].a_w.w_long)?SYM_TRUE->s_name:SYM_FALSE->s_name);
		}
		else if (a->j_f_type == J_BYTE) 	{
			a->getter((void *)env,x,a,&ac,&av);
			sprintf(str,"  %s [bool] %ld",aname,(long)av[0].a_w.w_long);
		}
		else if (a->j_f_type == J_CHAR) {	
			a->getter((void *)env,x,a,&ac,&av);
			sprintf(str,"  %s [char] %c",aname,(char)av[0].a_w.w_long);
		}
		else if (a->j_f_type == J_SHORT) {
			a->getter((void *)env,x,a,&ac,&av);
			sprintf(str,"  %s [short] %ld",aname,(long)av[0].a_w.w_long);
		}
		else if (a->j_f_type == J_INT_ARRAY)
			sprintf(str,"  %s [int array] ...  ",aname);
		else if (a->j_f_type == J_FLOAT_ARRAY)
			sprintf(str,"  %s [float array] ...",aname);
		else if (a->j_f_type == J_STRING_ARRAY)
			sprintf(str,"  %s [string array] ...",aname);
		else if (a->j_f_type == J_LONG_ARRAY)
			sprintf(str,"  %s [long array] ...  ",aname);
		else if (a->j_f_type == J_DOUBLE_ARRAY)
			sprintf(str,"  %s [double  array] ...",aname);
		else if (a->j_f_type == J_BOOL_ARRAY)
			sprintf(str,"  %s [bool array] ...",aname);
		else if (a->j_f_type == J_BYTE_ARRAY)
			sprintf(str,"  %s [byte array] ...",aname);
		else if (a->j_f_type == J_CHAR_ARRAY)
			sprintf(str,"  %s [char array] ...",aname);
		else if (a->j_f_type == J_SHORT_ARRAY)
			sprintf(str,"  %s [short array] ...",aname);		
		else 
			sprintf(str,"%s [ %s ]",aname,a->j_f_type->s_name);
		
		items[c_idx++] = gensym(str);
		if (av && ac)
			mxj_freebytes(av, ac * sizeof(t_atom));
	}
	
	items[c_idx++] = gensym("MESSAGES");
	str[0] = '\0';
	for(i = 0;i < x->methodlist->size;i++) {
		char *mname;
		t_mxj_method *m = x->methodlist->mlist[i];
		mname = m->name->s_name;
		for(p = 0; p < m->sigcount;p++) {
			char sigstr[256];
			char *jp_sig;
			long siglen;
			
			sigstr[0] = '[';
			sigstr[1] = '\0';
			
			jp_sig = m->jp_types[p]->s_name;
			siglen = (long)strlen(jp_sig);
			
			for(q = 0; q < siglen;q++) {
				switch(jp_sig[q]) {
					case 'G':
						strcat(sigstr,"Atom[]");	
						break;
					case 'I':
						strcat(sigstr,"int");
						break;
					case 'F':
						strcat(sigstr,"float");
						break;
					case 's':
						strcat(sigstr,"String");
						break;
					case 'V':
						//there can not be any more parameters if it is void
						sigstr[0] = '\0';
						break;
					case 'Z':
						strcat(sigstr,"bool");
						break;
					case 'B':
						strcat(sigstr,"byte");
						break;
					case 'C':
						strcat(sigstr,"char");
						break;
					case 'S':
						strcat(sigstr,"short");
						break;
					case 'J':
						strcat(sigstr,"long");
						break;
					case 'D':
						strcat(sigstr,"double");
						break;
					case '[':
						switch(jp_sig[q + 1]) {
							case 'I':
								strcat(sigstr,"int[]");
								break;
							case 'F':
								strcat(sigstr,"float[]");
								break;
							case 's':
								strcat(sigstr,"String[]");
								break;
							case 'V':
								strcat(sigstr,"void");
								break;
							case 'Z':
								strcat(sigstr,"bool[]");
								break;
							case 'B':
								strcat(sigstr,"byte[]");
								break;
							case 'C':
								strcat(sigstr,"char[]");
								break;
							case 'S':
								strcat(sigstr,"short[]");
								break;
							case 'J':
								strcat(sigstr,"long[]");
								break;
							case 'D':
								strcat(sigstr,"double[]");
								break;
							default:
								strcat(sigstr,"!unknown array type!");
								break;
						}
						q++;
						break;												
				
				default:
					strcat(sigstr,"!unknown param type!");
					break;		
				}	
				if (q != siglen - 1)
					strcat(sigstr,",");
			}
			if (sigstr[0] != '\0')	//is 0 when method is void
				strcat(sigstr,"]");
			sprintf(str,"  %s %s",mname,sigstr);
			items[c_idx++] = gensym(str);
		}
	}
	
	*numitems = c_idx;
}

void mxj_loadbang(t_maxjava *x)
{
	JNIEnv *env;
	static jmethodID mid = 0;
	THREADENV(env);
	if (!mid) {
		mid = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls,"loadbang", "()V");
		checkException(env);
	}
	MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance,mid);
	checkException(env);
}


void mxj_dblclick(t_maxjava *x)
{
	JNIEnv *env;
	static jmethodID mid = 0;
	THREADENV(env);
	if (!mid) {
		mid = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls,"dblclick", "()V");
		checkException(env);
	}
	
	MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance,mid);
	checkException(env);
}

	
void mxj_fileusage(t_maxjava *x, void *w)
{
	fileusage_addpackage(w, "max-mxj", NULL);
}
	

/////MESSAGE RESOLUTION/////////////////////////////////////////////////////////////


int maxjava_do_attr_get(t_maxjava *x, t_symbol *attr_name)
{
	JNIEnv *env;
	t_mxj_attr *a;
	THREADENV(env);
	
	if ((a = mxj_get_attr(x,attr_name)) != 0) {
			short ac;
			t_atom *av = 0;
			
			if (!x->nfo_outlet) {
				int i;
				a->getter((void *)env,x,a,&ac,&av);
				post("%s: ",attr_name);
				for(i = 0; i < ac;i++) { 
					switch(av[i].a_type) {
						case A_LONG:
							post("%d",av[i].a_w.w_long);
							break;
						case A_FLOAT:
							post("%f",av[i].a_w.w_float);
							break;
						case A_SYM:
							post("%s",av[i].a_w.w_sym->s_name);
							break;
					}
				}
				
				mxj_freebytes(av, ac * sizeof(t_atom));
				return 1;
			}
		
			a->getter((void *)env,x,a,&ac,&av);
			if (ac && av) {
				outlet_anything(x->nfo_outlet,attr_name,ac,av);
				mxj_freebytes(av, ac * sizeof(t_atom));
				return 1;
			} else {	// this could happen when an attribute is null 
				outlet_anything(x->nfo_outlet,attr_name,0,av);
				return 1;
			}	
	
	}
	
	return 0;	// no attr named attr_name found
}

void maxjava_handle_get(t_maxjava *x, t_symbol *msg, short argc, t_atom *argv)  
{
	JNIEnv *env;
	t_symbol* attr_name;
	short ac;
	t_atom *av;
	t_mxj_attr *a;
	
	THREADENV(env);
	ac = 0;
	av = 0;
	
	if (!x->nfo_outlet) {
		//probably want to do something more intelligent later like post to console
		post("(mxj) no info outlet");
		return;
	}
	if (argc) {
		attr_name = argv[0].a_w.w_sym;
		if ((a = mxj_get_attr(x,attr_name)) != 0) {
			//do get
			a->getter((void *)env,x,a,&ac,&av);
			
			if (ac && av) {
				if (av[0].a_type != A_SYM)
					outlet_list(x->nfo_outlet,0L,ac,av);
				else
					if (ac == 1)
						outlet_anything(x->nfo_outlet,av[0].a_w.w_sym,0,0L);
					else					
						outlet_anything(x->nfo_outlet,av[0].a_w.w_sym, ac - 1, (av + 1));
			
				mxj_freebytes(av, ac * sizeof(t_atom));
				return;
			} else {	// this could happen when an array or string is not set yet and never initialized.
				outlet_int(x->nfo_outlet,0);
			}
		} else {
			error("(mxj) get: No attribute named %s.",attr_name->s_name);
		}
	} else
		error("(mxj) get what???");
	
	return;	
}

void maxjava_anything(t_maxjava *x, t_symbol *msg, short argc, t_atom *argv)
{
	JNIEnv *env;
	
	t_mxj_method *m = NULL;
	char *p_types;
	char *ps;
	int i,resolve_stat = 0, meth_offset;
	jobject s,tmp;
	
	THREADENV(env);
	
	ps = msg->s_name;
	
	if ( ps[0]  == 'g' && ps[1]  == 'e' && ps[2]  == 't') {
		ps+=3;
		if (maxjava_do_attr_get(x,gensym(ps)))
			return;
	}
		
	p_types = mxj_getbytes(argc+2);	
	p_types[0] = '\0';
	
	if (argc == 0){
		p_types[0] = 'V';
		p_types[1] = '\0';
	
	} else {	// build param types of incoming message. Can only be one of 3 things in max rite now.
		for(i = 0; i < argc; i++) {
			switch (argv[i].a_type) {
				case A_LONG:
					p_types[i] = 'I';
					break;
				case A_FLOAT:
					p_types[i] = 'F';
					break;
				case A_SYM:
					p_types[i] = 's';
					break;
				default:
					break;
			}
		}
		
		p_types[i] = '\0';
	}
	
	if (x->methodlist)
		m = mxj_methodlist_resolve(x->methodlist,msg->s_name,argc,p_types,&resolve_stat,&meth_offset);

	mxj_freebytes(p_types,argc +2);
	
	MXJ_JNI_CALL(env,ExceptionClear)(env);
	
	switch (resolve_stat) {
		t_mxj_attr *a;
		case MXJ_METHOD_MATCH:
		case MXJ_ARRAY_MATCH:	
		case MXJ_COERCE:
		case MXJ_GIMME_MATCH:
			call_method_with_coercion(x,m->mids[meth_offset],m->jp_types[meth_offset],argc,argv);
			break;
		case MXJ_NO_METHOD_MATCH: 
			//look for attribute
			if ((a = mxj_get_attr(x,msg)) != 0) {	// should probbly check settable here
				a->setter((void *)env,x,a,argc,argv);
				return;
			}

			//call anything
			s = (jobject )MXJ_JNI_CALL(env,NewStringUTF)(env, msg->s_name);
			tmp = newAtomArray(env, argc, argv);
			
			//should we coerce calls to anything -toph?
			MXJ_JNI_CALL(env,CallVoidMethod)(env, x->javaInstance, x->inlet_anything_MID,s,tmp);
			// OK even though this shouldn't be the case this is very important. When creating a new Object and passing
			// it into a java method to be operated on. You must manually free the local reference because for some reason
			// the JVM cannot even though it is documented that it should!! This results in a very slow memory leak.
			MXJ_JNI_CALL(env,DeleteLocalRef)(env,s);
			MXJ_JNI_CALL(env,DeleteLocalRef)(env,tmp);
			break;
		default:
			break;
	}
	
	if (MXJ_JNI_CALL(env,ExceptionCheck)(env)) {
		DBG(post("exception!"));
		MXJ_JNI_CALL(env,ExceptionDescribe)(env);
	}
}

/* Does coercion if necessary..*/
void call_method_with_coercion(t_maxjava *x, jmethodID mid, t_symbol *jp_types, short argc, t_atom *argv)
{
	jvalue args[257];
	jobject str_free[64];
	jobject js;
	int num_strs_free = 0;
	char *sigstr;
	long i = 0,siglen = 0;
	t_symbol *s;
	jobject tmp;
	JNIEnv *env;
	THREADENV(env);
	
	//the signature is our java signature which has types z,b,c,s,i,l,f,d,S(string),and G(gimme)
	//as opposed to our max signature which only has types I,F,S,G.

	sigstr = jp_types->s_name;
	siglen = (long)strlen(sigstr);
	
	for (i=0;i<siglen;i++)
	{
		switch (sigstr[i]) {		
		case 'G':	
			tmp = newAtomArray(env, argc, argv);
			args[i].l = tmp;				
			MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,args);
			MXJ_JNI_CALL(env,DeleteLocalRef)(env,tmp);
			checkException(env);
			return;	
		case 'I': //int
			args[i].i = (jint) atomargs_getlong(i,argc,argv);
			break;
		case 'F': //float
			args[i].f = (jfloat) atomargs_getfloat(i,argc,argv);
			break;
		case 's': //string object, note the use of lowercase 's' for the java sig
			s = atomargs_getsym(i,argc,argv);
			js = (jobject )MXJ_JNI_CALL(env,NewStringUTF)(env, s->s_name);
			args[i].l = js;  
			str_free[num_strs_free] = js;//this is so we can do a deletelocalref after execution.
			num_strs_free++;              //it seems that vm has a problem freeing stuff we create and pass into it!
			break;		
		case 'V': //void p_types
			break;
		case 'Z': //boolean
			args[i].z = (jboolean) atomargs_getboolean(i,argc,argv); 
			break;
		case 'B': //byte
			args[i].b = (jbyte) atomargs_getlong(i,argc,argv); 
			break;
		case 'C': //char
			args[i].c = (jchar) atomargs_getchar(i,argc,argv);
			break;
		case 'S': //short
			args[i].s = (jshort) atomargs_getlong(i,argc,argv);
			break;
		
		case 'J': //long
			args[i].j = (jlong) atomargs_getlong(i,argc,argv);
			break;

		case 'D': //double
			args[i].d = (jdouble) atomargs_getfloat(i,argc,argv);
			break;
		case '[': //method is expeciting primative array, it is our responsibility to make sure sigstr + 1 exists and is a known primative type
			switch(sigstr[i + 1]) 	{
				int q;
				jvalue array_arg;
				jarray ja;
				jstring js;
				void *vp;
				
				case 's': //string object, note the use of lowercase 's' for the java sig
					ja  = (jarray)MXJ_JNI_CALL(env,NewObjectArray)(env,argc,g_stringClass,NULL);
					checkException(env);
									
					for(q = 0;q < argc;q++)
					{
						js = (MXJ_JNI_CALL(env,NewStringUTF)(env,(atomargs_getsym(q,argc,argv))->s_name));
						MXJ_JNI_CALL(env,SetObjectArrayElement)(env,ja,q,js);  	
						MXJ_JNI_CALL(env,DeleteLocalRef)(env,js);
					}
					checkException(env);
					
					array_arg.l = ja;				
					MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					checkException(env);
					return;						
				case 'I': //int array
					vp  = (jint *)mxj_getbytes(argc * sizeof(jint));
					ja  = (jarray)MXJ_JNI_CALL(env,NewIntArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jint *)vp)[q] = (jint)atomargs_getlong(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetIntArrayRegion)(env,(jintArray)ja,0,argc,(jint *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,&array_arg);
					checkException(env);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					mxj_freebytes(vp, argc * sizeof(jint));
					return;		
				case 'F': //float array
					vp  = (jfloat *)mxj_getbytes(argc * sizeof(jfloat));
					ja  = (jarray)MXJ_JNI_CALL(env,NewFloatArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jfloat *)vp)[q] = (jfloat)atomargs_getfloat(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetFloatArrayRegion)(env,(jfloatArray)ja,0,argc,(jfloat *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,&array_arg);
					checkException(env);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					mxj_freebytes(vp, argc * sizeof(jfloat));	
					return;		
				case 'J': //long array
					vp  = (jlong *)mxj_getbytes(argc * sizeof(jlong));
					ja  = (jarray)MXJ_JNI_CALL(env,NewLongArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jlong *)vp)[q] = (jlong)atomargs_getlong(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetLongArrayRegion)(env,(jlongArray)ja,0,argc,(jlong *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,&array_arg);
					checkException(env);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					mxj_freebytes(vp, argc * sizeof(jlong));
					return;		
				
				case 'D': //double array
					vp  = (jdouble *)mxj_getbytes(argc * sizeof(jdouble));
					ja  = (jarray)MXJ_JNI_CALL(env,NewDoubleArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jdouble *)vp)[q] = (jdouble)atomargs_getfloat(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetDoubleArrayRegion)(env,(jdoubleArray)ja,0,argc,(jdouble *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,&array_arg);
					checkException(env);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					mxj_freebytes(vp, argc * sizeof(jdouble));
					return;			
				case 'Z': //boolean array
					vp  = (jboolean *)mxj_getbytes(argc * sizeof(jboolean));
					ja  = (jarray)MXJ_JNI_CALL(env,NewBooleanArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jboolean *)vp)[q] = atomargs_getboolean(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetBooleanArrayRegion)(env,(jbooleanArray)ja,0,argc,(jboolean *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,&array_arg);
					checkException(env);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					mxj_freebytes(vp, argc * sizeof(jboolean));
					return;					
				case 'B': //byte array
					vp  = (jbyte *)mxj_getbytes(argc * sizeof(jbyte));
					ja  = (jarray)MXJ_JNI_CALL(env,NewByteArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jbyte *)vp)[q] = (jbyte)atomargs_getlong(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetByteArrayRegion)(env,(jbyteArray)ja,0,argc,(jbyte *)vp); 
					array_arg.l = ja;
					checkException(env);
					MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,&array_arg);
					checkException(env);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					mxj_freebytes(vp, argc * sizeof(jbyte));
					return;		
				case 'C': //char array
					vp  = (jchar *)mxj_getbytes(argc * sizeof(jchar));
					ja  = (jarray)MXJ_JNI_CALL(env,NewCharArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jchar *)vp)[q] = (jchar)atomargs_getchar(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetCharArrayRegion)(env,(jcharArray)ja,0,argc,(jchar *)vp); 
					array_arg.l = ja;
					checkException(env);
					MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,&array_arg);
					checkException(env);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					mxj_freebytes(vp, argc * sizeof(jchar));
					return;		
				case 'S': //short array
					vp  = (jshort *)mxj_getbytes(argc * sizeof(jshort));
					ja  = (jshortArray)MXJ_JNI_CALL(env,NewShortArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jshort *)vp)[q] = (jshort)atomargs_getlong(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetShortArrayRegion)(env,(jshortArray)ja,0,argc,(jshort *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid,&array_arg);
					checkException(env);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					mxj_freebytes(vp, argc * sizeof(jshort));
					return;		
				default:
					error("unsupported array type [%c",sigstr[i + 1]);
					return;
			
			}
			break;	
		//end case '['
		default:
			error("something in our signature is messed up");
			//should free all allocated string objects
			return;
		}
	}
	
	MXJ_JNI_CALL(env,CallVoidMethodA)(env, x->javaInstance, mid, args);
	for(i = 0; i < num_strs_free;i++)
		MXJ_JNI_CALL(env,DeleteLocalRef)(env,str_free[i]);

	if (MXJ_JNI_CALL(env,ExceptionCheck)(env)) {
		DBG(post("exception!"));
		MXJ_JNI_CALL(env,ExceptionDescribe)(env);
	}
}




/* Does coercian if necessary..*/
t_mxj_err call_constructor_with_coercion(t_maxjava *x, jmethodID mid, t_symbol *jp_types, short argc, t_atom *argv)
{
	jvalue args[257];
	jobject str_free[64];
	int num_strs_free = 0;
	jstring js;
	char *sigstr;
	long i = 0,siglen = 0, ret = MXJ_ERR_NONE;
	t_symbol *s;
	jobject tmp;
	//jthrowable excep;
	JNIEnv *env;
	
	THREADENV(env);

	//the signature is our java signature which has types z,b,c,s,i,l,f,d,S(string),and G(gimme)
	//as opposed to our max signature which only has types I,F,S,G.

	sigstr = jp_types->s_name;
	siglen = (long)strlen(sigstr);

	for (i=0; i<siglen; i++) {
		switch (sigstr[i]) {		
		case 'G':	
			tmp = newAtomArray(env, argc, argv);
			args[i].l = tmp;				
			MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,args);
			MXJ_JNI_CALL(env,DeleteLocalRef)(env,tmp);
			ret = constructor_excep_helper(env);
			return ret;	
		case 'I': // int
			args[i].i = (jint) atomargs_getlong(i,argc,argv);
			break;
		case 'F': // float
			args[i].f = (jfloat) atomargs_getfloat(i,argc,argv);
			break;
		case 's': // string object, note the use of lowercase 's' for the java sig
			s = atomargs_getsym(i,argc,argv);
			js = (jobject )MXJ_JNI_CALL(env,NewStringUTF)(env, s->s_name);
			args[i].l = js;  
			str_free[num_strs_free] = js;//this is so we can do a deletelocalref after execution.
			num_strs_free++;  
			break;		
		case 'V': // void p_types
			break;
		case 'Z': // boolean
			args[i].z = (jboolean) atomargs_getboolean(i,argc,argv); 
			break;
		case 'B': // byte
			args[i].b = (jbyte) atomargs_getlong(i,argc,argv); 
			break;
		case 'C': // char
			args[i].c = (jchar) atomargs_getchar(i,argc,argv);
			break;
		case 'S': // short
			args[i].s = (jshort) atomargs_getlong(i,argc,argv);
			break;
		
		case 'J': // long
			args[i].j = (jlong) atomargs_getlong(i,argc,argv);
			break;

		case 'D': // double
			args[i].d = (jdouble) atomargs_getfloat(i,argc,argv);
			break;
		case '[': // method is expeciting primitive array, it is our responsibility to make sure sigstr + 1 exists and is a known primitive type
			switch(sigstr[i + 1])
			{
				int q;
				jvalue array_arg;
				jarray ja;
				jstring js;
				void *vp;
				
				case 's': // string object, note the use of lowercase 's' for the java sig
					ja  = (jarray)MXJ_JNI_CALL(env,NewObjectArray)(env,argc,g_stringClass,NULL);
					checkException(env);
									
					for(q = 0;q < argc;q++)
					{
						js = (MXJ_JNI_CALL(env,NewStringUTF)(env,(atomargs_getsym(q,argc,argv))->s_name));
						MXJ_JNI_CALL(env,SetObjectArrayElement)(env,ja,q,js);  	
						MXJ_JNI_CALL(env,DeleteLocalRef)(env,js);
					}
					checkException(env);
					
					array_arg.l = ja;				
					MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					ret = constructor_excep_helper(env);
					return ret;						
				case 'I': //int array
					vp  = (jint *)mxj_getbytes(argc * sizeof(jint));
					ja  = (jarray)MXJ_JNI_CALL(env,NewIntArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jint *)vp)[q] = (jint)atomargs_getlong(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetIntArrayRegion)(env,(jintArray)ja,0,argc,(jint *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					ret = constructor_excep_helper(env);
					mxj_freebytes(vp, argc * sizeof(jint));
					return ret;		
				case 'F': //float array
					vp  = (jfloat *)mxj_getbytes(argc * sizeof(jfloat));
					ja  = (jarray)MXJ_JNI_CALL(env,NewFloatArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jfloat *)vp)[q] = (jfloat)atomargs_getfloat(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetFloatArrayRegion)(env,(jfloatArray)ja,0,argc,(jfloat *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					ret = constructor_excep_helper(env);
					mxj_freebytes(vp, argc * sizeof(jfloat));
					return ret;		
				case 'J': //long array
					vp  = (jlong *)mxj_getbytes(argc * sizeof(jlong));
					ja  = (jarray)MXJ_JNI_CALL(env,NewLongArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jlong *)vp)[q] = (jlong)atomargs_getlong(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetLongArrayRegion)(env,(jlongArray)ja,0,argc,(jlong *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					ret = constructor_excep_helper(env);
					mxj_freebytes(vp, argc * sizeof(jlong));
					return ret;		
				
				case 'D': //double array
					vp  = (jdouble *)mxj_getbytes(argc * sizeof(jdouble));
					ja  = (jarray)MXJ_JNI_CALL(env,NewDoubleArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jdouble *)vp)[q] = (jdouble)atomargs_getfloat(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetDoubleArrayRegion)(env,(jdoubleArray)ja,0,argc,(jdouble *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					ret = constructor_excep_helper(env);
					mxj_freebytes(vp, argc * sizeof(jdouble));
					return ret;			
				case 'Z': //boolean array
					vp  = (jboolean *)mxj_getbytes(argc * sizeof(jboolean));
					ja  = (jarray)MXJ_JNI_CALL(env,NewBooleanArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jboolean *)vp)[q] = atomargs_getboolean(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetBooleanArrayRegion)(env,(jbooleanArray)ja,0,argc,(jboolean *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					ret = constructor_excep_helper(env);
					mxj_freebytes(vp, argc * sizeof(jboolean));
					return ret;					
				case 'B': //byte array
					vp  = (jbyte *)mxj_getbytes(argc * sizeof(jbyte));
					ja  = (jarray)MXJ_JNI_CALL(env,NewByteArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jbyte *)vp)[q] = (jbyte)atomargs_getlong(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetByteArrayRegion)(env,(jbyteArray)ja,0,argc,(jbyte *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					ret = constructor_excep_helper(env);
					mxj_freebytes(vp, argc * sizeof(jbyte));
					return ret;		
				case 'C': //char array
					vp  = (jchar *)mxj_getbytes(argc * sizeof(jchar));
					ja  = (jarray)MXJ_JNI_CALL(env,NewCharArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jchar *)vp)[q] = (jchar)atomargs_getchar(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetCharArrayRegion)(env,(jcharArray)ja,0,argc,(jchar *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
					ret = constructor_excep_helper(env);
					mxj_freebytes(vp, argc * sizeof(jchar));
					return ret;		
				case 'S': //short array
					vp  = (jshort *)mxj_getbytes(argc * sizeof(jshort));
					ja  = (jshortArray)MXJ_JNI_CALL(env,NewShortArray)(env,argc);
					checkException(env);
									
					for(q = 0;q < argc;q++)
						((jshort *)vp)[q] = (jshort)atomargs_getlong(q,argc,argv); 	
					
					MXJ_JNI_CALL(env,SetShortArrayRegion)(env,(jshortArray)ja,0,argc,(jshort *)vp); 
					array_arg.l = ja;
					checkException(env);
					
					MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,&array_arg);
					MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
	
					ret = constructor_excep_helper(env);
					mxj_freebytes(vp, argc * sizeof(jshort));
					return ret;		
				default:
					error("unsupported array type [%c",sigstr[i + 1]);
					return MXJ_ERR_GENERIC;
			
			}
			break;	
		//end case '['
		default:
			error("something in our signature is messed up");
			//should free all allocated string objects
			return MXJ_ERR_GENERIC;
		}
	}
	
	MXJ_JNI_CALL(env,CallNonvirtualVoidMethodA)(env,x->javaInstance,x->cls,mid,args);

	ret = constructor_excep_helper(env);
	
	for(i = 0; i < num_strs_free;i++)
		MXJ_JNI_CALL(env,DeleteLocalRef)(env,str_free[i]);
	
	return ret;
}

t_mxj_err constructor_excep_helper(JNIEnv *env)
{
	jthrowable excep;
	
	if ((excep = MXJ_JNI_CALL(env,ExceptionOccurred)(env)) != NULL) {
		if (MXJ_JNI_CALL(env,IsInstanceOf)(env, excep, g_max_runtime_exception_clz)) {
			MXJ_JNI_CALL(env,ExceptionClear)(env);	// don't print anything if it is a MaxRuntime exception
			return MXJ_ERR_BAIL;					// we assume that the programmer has printed the error
		}											
		else {
			MXJ_JNI_CALL(env,ExceptionDescribe)(env);	
			return MXJ_ERR_GENERIC;
		}
	}
	else
		return MXJ_ERR_NONE;
}

/* returns the mxj_attr obj * if it finds one */
t_mxj_attr* mxj_get_attr(t_maxjava *x,t_symbol *name)
 {
 	int i;
 	for(i = 0; i < x->attr_num;i++) {
 		t_mxj_attr *a;
 		a = x->attrlist[i];
 		if (name == a->name)
 			return a;
 	}
	 
 	return 0;
 }
 

jlong atomargs_getlong(long idx, short ac, t_atom *av)
{	
	jlong v=0;
	
	if (ac && av && (idx<ac)) {
		switch (av[idx].a_type)
		{
			case A_LONG: 
			 	v = av[idx].a_w.w_long;  
			 	break;
			case A_FLOAT: 
				v = (jlong)av[idx].a_w.w_float;
				break;
			case A_SYM:
				if (av[idx].a_w.w_sym == SYM_TRUE)
					return  1;
				else if (av[idx].a_w.w_sym == SYM_FALSE)
					return 0;	 
		 		post("(mxj warning)strange coercion: max symbol forced to numeric. All non-empty symbols except 'false' are 1 on this context.");
		 		v = (av[idx].a_w.w_sym->s_name[0])?1:0;
				break;
		}
	}
	return v;	
}

jdouble atomargs_getfloat(long idx, short ac, t_atom *av)
{	
	jdouble v=0.;
	if (ac && av && (idx<ac)) {
		switch (av[idx].a_type)
		{
		case A_FLOAT: v = av[idx].a_w.w_float; break;
		case A_LONG:  v = (jdouble)(av[idx].a_w.w_long);  break;
		case A_SYM: 
				if (av[idx].a_w.w_sym == SYM_TRUE)
					return 1.0;
				else if (av[idx].a_w.w_sym == SYM_FALSE)
					return 0.0;	
				post("(mxj warning)strange coercion: max symbol forced to numeric. All non-empty symbols except 'false' are 1 on this context.");
			 	v = (av[idx].a_w.w_sym->s_name[0])?1:0;
			 	break;
		
		}
	}
	
	return v;	
}

t_symbol *atomargs_getsym(long idx, short ac, t_atom *av)
{	
	t_symbol *v;
	static t_symbol *ps_nothing=NULL;
	char buf[16];
	
	if (!ps_nothing) {
		//the symbol for the empty string
		ps_nothing = gensym("");
	}
	
	v = ps_nothing;
	
	if (ac&&av&&(idx<ac)) {
		switch (av[idx].a_type)
		{
		case A_SYM: 
			v = av[idx].a_w.w_sym;
			break;
		case A_LONG:
			post("(mxj warning)strange coercion: max numeric (long) forced to symbol. ");
			sprintf(buf,"%ld",(long)av[idx].a_w.w_long);
			return(gensym(buf));
		case A_FLOAT:
			post("(mxj warning)strange coercion: max numeric (float) forced to symbol. ");
			sprintf(buf,"%f",av[idx].a_w.w_float);
			return(gensym(buf));
			
		}
	}
	
	return v;	
}

jchar atomargs_getchar(long idx, short ac, t_atom *av)
{	
	jchar v=0;
	if (ac && av && (idx<ac)) {
		switch (av[idx].a_type)
		{
		case A_LONG:  
			v = (jchar)av[idx].a_w.w_long;
		 	break;
		case A_FLOAT: 
			v = (jchar)av[idx].a_w.w_float;
			break;
		case A_SYM:
			if (av[idx].a_w.w_sym->s_name[1] != '\0')
				post("(mxj warning)strange coercion: max symbol forced to char. First char is used. ");
			v = av[idx].a_w.w_sym->s_name[0];
			break;
		}
	}
	
	return v;	
}


jboolean atomargs_getboolean(long idx, short ac, t_atom *av)
{	
	jboolean v=0;
	
	if (ac && av && (idx<ac)) {
		switch (av[idx].a_type)
		{
		case A_SYM:
			v = (av[idx].a_w.w_sym == SYM_FALSE) ? JNI_FALSE : JNI_TRUE;
			break;
		case A_LONG:  
			v = (av[idx].a_w.w_long) ? JNI_TRUE : JNI_FALSE;
		 	break;
		case A_FLOAT: 
			v = (av[idx].a_w.w_float) ? JNI_TRUE : JNI_FALSE;
			break;
		}
	}
	
	return v;	
}



//
// Assistance to the user.
//
void maxjava_assist(t_maxjava *x, void *box, long msgType, long argNum, char *dest)
{
	jmethodID mid = NULL;
	jstring assistStr = NULL;
	const char *utf;
	int utfLen, copyLen;
	JNIEnv *env;
	
	THREADENV(env);
	
	if (msgType == ASSIST_INLET) {
#ifdef MXJ_MSP
		if (x->m_nInlets == 0 && x->num_sig_inlets == 0) {
			strcpy(dest,"msp default inlet");
			return;
		}
#endif
	    mid = x->getInletAssist_MID;
	} 
	else if (msgType == ASSIST_OUTLET) {
		if (argNum == x->m_nOutlets - 1 && x->nfo_outlet) { // nfo outlet
			strcpy(dest,"info outlet");
			return;
		}
	    mid = x->getOutletAssist_MID;
	} else {
		return;
	}

	MXJ_JNI_CALL(env,ExceptionClear)(env);
	assistStr = MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, mid, (int)argNum);
	if (MXJ_JNI_CALL(env,ExceptionCheck)(env)) {
		DBG(post("exception!"));
		MXJ_JNI_CALL(env,ExceptionDescribe)(env);
	}
	
	utf = MXJ_JNI_CALL(env,GetStringUTFChars)(env, assistStr, NULL);
	utfLen = MXJ_JNI_CALL(env,GetStringLength)(env, assistStr);
	copyLen = (utfLen < ASSIST_MAX) ? utfLen : ASSIST_MAX;
	strncpy(dest, utf, copyLen);
	dest[copyLen] = '\0';
	
	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, assistStr, utf);
	MXJ_JNI_CALL(env,DeleteLocalRef)(env, assistStr);
}

///// MSP................................................................ 

#ifdef MXJ_MSP
void mxj_dsp_halt(t_maxjava *x)
{
	t_object *dspobj;
	dspobj = ps_dsp->s_thing;
	if (dspobj)
		mess0(dspobj,ps_stop);
}

void mxj_bounce_dac()
{
	t_object *dspobj;
	dspobj = ps_dsp->s_thing;
	//restart dac to build chain so we can add bm perfrom method
	if (dspobj) {
		mess0(dspobj,ps_stop);
		mess1(dspobj,ps_start,0);		
	}
}

void mxj_benchmark(t_maxjava *x, long way, long interval )
{
	
	if (way){
		x->benchmark_perform = 1;
		x->benchmark_loops = 0;
		x->benchmark_total = 0;
		x->benchmark_best_time = 77777777;
		x->benchmark_worst_time = 0;
		if (interval && interval > 0)
			x->benchmark_interval = interval;
		else
			x->benchmark_interval = 256;
		
		if (x->perf_exception_check) {
			x->perf_exception_check = 0;
			post("(mxj~ %s) exceptioncheck switched off to enable benchmark",x->p_classname);
		}
	}
	else{
		x->benchmark_perform = 0;
	}
	if (!x->dsp_running)
		return;
	else
		mxj_bounce_dac();
}

void mxj_exception_check(t_maxjava *x,int way)
{
	if (way) {
		x->perf_exception_check = 1;
		if (x->benchmark_perform) {
			x->benchmark_perform = 0;
			post("(mxj~ %s) benchmark switched off to enable exceptioncheck",x->p_classname);
		}
	}
	else
		x->perf_exception_check = 0;	
	if (!x->dsp_running)
		return;
	else
		mxj_bounce_dac();		
}
	
//BEGIN DSP ROUTINE
void mxj_dsp64(t_maxjava *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
#ifdef C74_X64
	static short mode = MXJ_MSP_MODE_COMPATIBLE;
#else
	static short mode = MXJ_MSP_MODE_HIPERFORMANCE;
#endif
	static int firsttime = 1;

	if (firsttime)
	{
		mode = (short)(t_ptr_int)mxj_get_prop_val(MXJPROP_MSP_MODE,(void*)(t_ptr_int)mode);
		firsttime = 0;
	}
	
	mxj_dsp_add64(x, dsp64, count, samplerate, maxvectorsize, flags, mode);
}

void mxj_dsp_add64(t_maxjava *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags, short mode) {
	jobject jobj;
	int i,nsi,nso;
	JNIEnv *env;
	
	THREADENV(env);
	
	x->dsp_running = 1;
	nsi = x->num_sig_inlets;
	nso = x->num_sig_outlets;
	
	x->dsp_call_cnt++;
	if (x->dsp_call_cnt == 1)//we need to setup on the first call into dsp
	{
		//all this stuff is freed in maxjava_free
		if (nsi > 0)
		{
			x->inlet_msp_sigs = (jobject *)mxj_getbytes(sizeof(jobject) * nsi);
			x->inlet_msp_vecs = (jobject *)mxj_getbytes(sizeof(jfloatArray) * nsi);
		}
		if (nso > 0)
		{
			x->outlet_msp_sigs = (jobject *)mxj_getbytes(sizeof(jobject) * nso);
			x->outlet_msp_vecs = (jobject *)mxj_getbytes(sizeof(jfloatArray) * nso);
		}
		//create MSPSig[] inlets even if it is an empty array
		jobj = MXJ_JNI_CALL(env,NewObjectArray)(env,nsi,g_msp_sig_clz,NULL);
		x->msp_sig_inlets = MXJ_JNI_CALL(env,NewGlobalRef)(env,jobj);
		MXJ_JNI_CALL(env,DeleteLocalRef)(env,jobj);
		//create MSPSig[] outlets even if it is an empty array
		jobj = MXJ_JNI_CALL(env,NewObjectArray)(env,nso,g_msp_sig_clz,NULL);
		x->msp_sig_outlets = MXJ_JNI_CALL(env,NewGlobalRef)(env,jobj);
		MXJ_JNI_CALL(env,DeleteLocalRef)(env,jobj);
	}
	for(i = 0; i < nsi;i++)
	{
		//free references
		if (x->dsp_call_cnt > 1) {
			MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->inlet_msp_sigs[i]);
			MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->inlet_msp_vecs[i]);
		}
		
		//Make MSPSignal float[] member
		jobj =  MXJ_JNI_CALL(env,NewFloatArray)(env, maxvectorsize);
		x->inlet_msp_vecs[i] = MXJ_JNI_CALL(env,NewGlobalRef)(env, jobj);
		MXJ_JNI_CALL(env,DeleteLocalRef)(env, jobj);
		
		//Make MSPSignal for inlet member
		jobj = MXJ_JNI_CALL(env,NewObject)(env, g_msp_sig_clz, g_msp_sig_const,x->inlet_msp_vecs[i],(jdouble)samplerate,(jint)maxvectorsize,(jshort)count[i]);
		x->inlet_msp_sigs[i] = MXJ_JNI_CALL(env,NewGlobalRef)(env, jobj);
		MXJ_JNI_CALL(env,DeleteLocalRef)(env, jobj);
		
		//set MSPSignal[] inlets element
		MXJ_JNI_CALL(env,SetObjectArrayElement)(env,x->msp_sig_inlets,i,x->inlet_msp_sigs[i]);
	}

	for(i = 0; i < nso ;i++)
	{
		if (x->dsp_call_cnt > 1) {
			MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->outlet_msp_sigs[i]);
			MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->outlet_msp_vecs[i]);
		}
				
		//Make MSPSignal float[] member
		jobj =  MXJ_JNI_CALL(env,NewFloatArray)(env, maxvectorsize);
		x->outlet_msp_vecs[i] = MXJ_JNI_CALL(env,NewGlobalRef)(env, jobj);
		MXJ_JNI_CALL(env,DeleteLocalRef)(env, jobj);
		
		//Make MSPSignal for outlet member
		jobj = MXJ_JNI_CALL(env,NewObject)(env, g_msp_sig_clz, g_msp_sig_const,x->outlet_msp_vecs[i],(jdouble)samplerate,(jint)maxvectorsize,(jshort)(*(count+nsi+i)));
		x->outlet_msp_sigs[i] = MXJ_JNI_CALL(env,NewGlobalRef)(env, jobj);
		MXJ_JNI_CALL(env,DeleteLocalRef)(env, jobj);
		
		//set MSPSignal[] outlets element
		MXJ_JNI_CALL(env,SetObjectArrayElement)(env,x->msp_sig_outlets,i,x->outlet_msp_sigs[i]);
	}
	
	//now call java dsp method
	jobj = MXJ_JNI_CALL(env,CallObjectMethod)(env,x->javaInstance,x->dsp_MID,x->msp_sig_inlets,x->msp_sig_outlets);
	
	if (!jobj)//don't add anything to signal chain if null is returned from java dsp method
		return;
	
	if (checkException(env)) {
		post("(mxj~) exception in dsp");
		return;//maybe add signal pass here
	}

	x->sig_perform_MID = MXJ_JNI_CALL(env,FromReflectedMethod)(env,jobj);
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,jobj);
	
	if (checkException(env)) {
		post("(mxj~) exception in dsp");
		return;//maybe add signal pass here
	}
	
	//Do they want to benchmark their perform routine??
	if (x->benchmark_perform) {
		x->benchmark_loops = 0;
		x->benchmark_total = 0;
		x->benchmark_best_time = 77777777;
		x->benchmark_worst_time = 0;
		if (mode == MXJ_MSP_MODE_COMPATIBLE) {
			dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)mxj_perform_compatible_benchmark64, 0, NULL);
		} else {
			dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)mxj_perform_hiperformance_benchmark64, 0, NULL);
		}
	} //Do they want to check for exceptions in perform routine?
	else if (x->perf_exception_check) {
		x->perf_exception_occurred = 0;
		
		if (mode == MXJ_MSP_MODE_COMPATIBLE) {
			dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)mxj_perform_compatible_exception_check64, 0, NULL);
		} else {
			dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)mxj_perform_hiperformance_exception_check64, 0, NULL);
		}
	}
	else {
		//unroll this at some point and see if it still works so we do no pointer derefs.
		if (mode == MXJ_MSP_MODE_COMPATIBLE) {
			dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)mxj_perform_compatible64, 0, NULL);
		} else {
			dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)mxj_perform_hiperformance64, 0, NULL);
		}
	}
}
////END DSP ROUTINES

//BEGIN PERFORM ROUTINES

// these macros are needed to copy data to/from sig->vec to MSPSignal vec float[] member
// since the Max java signal processing for now is done with 32 bit floats we need to
// convert from/to the MSP native double signal processing
#define PERF_FLATTEN_HEADER	\
	int i,j;\
	double *dp; \
	jfloat* fp;	\
	jboolean iscopy; \
	JNIEnv *env;	\

#define PERF_FAST_FLATTEN_HEADER	\
	int i,j;\
	double *dp; \
	jfloat* fp;	\
	JNIEnv *env;	\

#define PERF_FLATTEN_ENTER(x) \
	env = x->aenv;	\
	for(i = 0; i < numins;i++)	\
	{										\
		dp = ins[i];	\
		fp = MXJ_JNI_CALL(env,GetPrimitiveArrayCritical)(env,x->inlet_msp_vecs[i],&iscopy);	\
		for (j=0; j<sampleframes; j++) {	\
			*fp++ = (jfloat)*dp++;	\
		} \
		MXJ_JNI_CALL(env,ReleasePrimitiveArrayCritical)(env,x->inlet_msp_vecs[i],fp,0);	\
	}	\

#define PERF_FLATTEN_EXIT(x)	\
	for(i = 0; i < numouts;i++)	\
	{	\
		dp = outs[i];	\
		fp = MXJ_JNI_CALL(env,GetPrimitiveArrayCritical)(env,x->outlet_msp_vecs[i],&iscopy);	\
		sysmem_copyptr(fp,outs[i],sampleframes * sizeof(t_mxj_sample));			\
		for (j=0; j<sampleframes; j++) {	\
			*dp++ = (double)*fp++;	\
		} \
		MXJ_JNI_CALL(env,ReleasePrimitiveArrayCritical)(env,x->outlet_msp_vecs[i],fp,0);	\
	}	\

#define PERF_FAST_FLATTEN_ENTER(x) \
	env = x->aenv;	\
	for(i = 0; i < numins;i++)	\
	{										\
		void *markbits=0; \
		dp = ins[i];	\
		markbits = ((t_java_array_ref *)x->inlet_msp_vecs[i])->mark;\
		((t_java_array_ref *)x->inlet_msp_vecs[i])->mark = (void *)( (~((t_ptr_uint)(0x7))) & (t_ptr_uint)(((t_java_array_ref *)x->inlet_msp_vecs[i])->mark)); \
		fp = (float *)((*(char **)x->inlet_msp_vecs[i]) + sizeof(t_java_array_ref));	\
		for (j=0; j<sampleframes; j++) {	\
			*fp++ = (jfloat)*dp++;	\
		} \
		((t_java_array_ref *)x->inlet_msp_vecs[i])->mark = (void *)markbits; \
	}	\

#define PERF_FAST_FLATTEN_EXIT(x)	\
	for(i = 0; i < numouts;i++)	\
	{										\
		void *markbits=0; \
		dp = outs[i];	\
		markbits = ((t_java_array_ref *)x->outlet_msp_vecs[i])->mark;\
		((t_java_array_ref *)x->outlet_msp_vecs[i])->mark = (void *)( (~((t_ptr_uint)(0x7))) & (t_ptr_uint)(((t_java_array_ref *)x->outlet_msp_vecs[i])->mark)); \
		fp = (float *)((*(char **)x->outlet_msp_vecs[i]) + sizeof(t_java_array_ref));	\
		for (j=0; j<sampleframes; j++) {	\
			*dp++ = (double)*fp++;	\
		} \
		((t_java_array_ref *)x->outlet_msp_vecs[i])->mark = (void *)markbits; \
	}	\

void mxj_perform_compatible64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	PERF_FLATTEN_HEADER	
	if (x->p_ob.z_disabled)
		return;

	THREADENV(x->aenv);
	
	PERF_FLATTEN_ENTER(x)
	
	MXJ_JNI_CALL(x->aenv,CallVoidMethod)(x->aenv, x->javaInstance, x->sig_perform_MID,x->msp_sig_inlets,x->msp_sig_outlets);	
	
	PERF_FLATTEN_EXIT(x)
}

void mxj_perform_hiperformance64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	PERF_FAST_FLATTEN_HEADER
	
	if (x->p_ob.z_disabled)
		return;
		
	THREADENV(x->aenv);
	
	PERF_FAST_FLATTEN_ENTER(x)
	
	MXJ_JNI_CALL(x->aenv,CallVoidMethod)(x->aenv, x->javaInstance, x->sig_perform_MID,x->msp_sig_inlets,x->msp_sig_outlets);	
	
	PERF_FAST_FLATTEN_EXIT(x)
}

void mxj_perform_compatible_benchmark64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	PERF_FLATTEN_HEADER
	double t;
	if (x->p_ob.z_disabled)
		return;
		
	THREADENV(x->aenv);

	BENCHMARK_BEGIN;
	PERF_FLATTEN_ENTER(x)
	MXJ_JNI_CALL(x->aenv,CallVoidMethod)(x->aenv, x->javaInstance, x->sig_perform_MID,x->msp_sig_inlets,x->msp_sig_outlets);
	PERF_FLATTEN_EXIT(x)
	BENCHMARK_END;
	t = BENCHMARK_RESULT; // microseconds
	
	x->benchmark_total += t;
	x->benchmark_loops++;
	if (t < x->benchmark_best_time)
		x->benchmark_best_time = t;
	if (t > x->benchmark_worst_time)
		x->benchmark_worst_time = t;	
	if (!(x->benchmark_loops % x->benchmark_interval)) {
		post("(mxj~ benchmark %s )microsecs: perf_t: %.2f | total_iter: %d | avg_perf_t: %.2f | best_t: %.2f | worst_t: %.2f",
		x->p_classname,t,x->benchmark_loops, x->benchmark_total / x->benchmark_loops,x->benchmark_best_time,x->benchmark_worst_time);
		x->benchmark_best_time = 77777777;
		x->benchmark_worst_time = 0;
	}	
}

void mxj_perform_hiperformance_benchmark64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	PERF_FAST_FLATTEN_HEADER
	
	double t;
	if (x->p_ob.z_disabled)
		return;
		
	THREADENV(x->aenv);

	BENCHMARK_BEGIN;
	PERF_FAST_FLATTEN_ENTER(x)
	
	MXJ_JNI_CALL(x->aenv,CallVoidMethod)(x->aenv, x->javaInstance, x->sig_perform_MID,x->msp_sig_inlets,x->msp_sig_outlets);
	
	PERF_FAST_FLATTEN_EXIT(x)
	BENCHMARK_END;
	
	t = BENCHMARK_RESULT; // microseconds
	
	x->benchmark_total += t;
	x->benchmark_loops++;
	if (t < x->benchmark_best_time)
		x->benchmark_best_time = t;
	if (t > x->benchmark_worst_time)
		x->benchmark_worst_time = t;	
	if (!(x->benchmark_loops % x->benchmark_interval))
	{
		post("(mxj~ benchmark %s )microsecs: perf_t: %.2f | total_iter: %d | avg_perf_t: %.2f | best_t: %.2f | worst_t: %.2f",
		x->p_classname,t,x->benchmark_loops, x->benchmark_total / x->benchmark_loops,x->benchmark_best_time,x->benchmark_worst_time);
		x->benchmark_best_time = 77777777;
		x->benchmark_worst_time = 0;
	}	
}

void mxj_perform_compatible_exception_check64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	PERF_FLATTEN_HEADER
	
	if (x->p_ob.z_disabled || x->perf_exception_occurred)
		return;
	THREADENV(x->aenv);
	
	PERF_FLATTEN_ENTER(x)
	
	MXJ_JNI_CALL(x->aenv,CallVoidMethod)(x->aenv, x->javaInstance, x->sig_perform_MID,x->msp_sig_inlets,x->msp_sig_outlets);
	
	if (checkException(x->aenv)) {
		x->perf_exception_occurred = 1;
		defer_front((t_object *)x,(method)mxj_dsp_halt,0,0,0);		
	} else {
		PERF_FLATTEN_EXIT(x)
	}
}

void mxj_perform_hiperformance_exception_check64(t_maxjava *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	PERF_FAST_FLATTEN_HEADER

	if (x->p_ob.z_disabled || x->perf_exception_occurred)
		return;
	
	THREADENV(x->aenv);
	
	PERF_FAST_FLATTEN_ENTER(x)
	
	MXJ_JNI_CALL(x->aenv,CallVoidMethod)(x->aenv, x->javaInstance, x->sig_perform_MID,x->msp_sig_inlets,x->msp_sig_outlets);
	
	if (checkException(x->aenv))
	{
		x->perf_exception_occurred = 1;
		defer_front((t_object *)x,(method)mxj_dsp_halt,0,0,0);		
	} else {
		PERF_FAST_FLATTEN_EXIT(x)
	}
}
///END PERFORM ROUTINES

void mxj_dspstate(t_maxjava *x, long n)
{	

	JNIEnv *env;
	jboolean val;	
	THREADENV(env);
	
	x->dsp_running = n;
	val = (n)?JNI_TRUE:JNI_FALSE;
	//call into java dspstate method
	MXJ_JNI_CALL(env,CallVoidMethod)(env,x->javaInstance,x->dspstate_MID,val);
	checkException(env);
}

jboolean mxj_check_z_no_inplace(JNIEnv *env, t_maxjava *x)
{
	jfieldID fid = 0;
	jboolean val = JNI_FALSE;
	
	fid = MXJ_JNI_CALL(env,GetFieldID)(env, x->cls, "_z_no_inplace", "Z");	

	if (fid) {
		val = MXJ_JNI_CALL(env,GetBooleanField)(env, x->javaInstance, fid);
		checkException(env);
	}
	
	return val;
}


#endif
///END MSP................................................................


//
// Working with the JVM.
//
JNIEnv *jvm_new(long *exists) {
	JNIEnv *env = NULL;
	
	if (g_jvm == NULL) {
	    JavaVMInitArgs vmArgs;
        //be very generous, we can have max 256 options
	    JavaVMOption options[256];
	    int numOptions = 0;
	    int i;
		int err;
		char *ps;
	
		*exists = 0;
		
		if (ps_global_jvm->s_thing) {
			g_jvm = (JavaVM*)ps_global_jvm->s_thing;
			THREADENV(env);
			*exists = 1;
			return env;
		}
		
#ifdef WIN_VERSION
		if (mxj_platform_init()) {
			error("(mxj) mxj_platform_init failed. Could not initialize the Java Runtime Environment.");
			error("Please check your Java installation.  http://java.sun.com");
			return NULL;
		}
#endif
        //be generous 256 options ..
		err = mxj_get_jvmopts(options,&numOptions, 256);

		if (err != MAX_ERR_NONE)
			return NULL;
	
 		#ifdef MAC_VERSION
		if (g_java_jvm_version[0]) {
			post("(mxj) attempting to to set java version to %s",g_java_jvm_version);
			setenv("JAVA_JVM_VERSION", (char*)g_java_jvm_version,1);
		}
 		#endif // MAC_VERSION

	    vmArgs.version = JNI_VERSION_1_4;
	    vmArgs.nOptions = numOptions;
	    vmArgs.options = options;
	    vmArgs.ignoreUnrecognized = JNI_FALSE;
		
		// classpath is in first option
		ps = strstr(options[0].optionString, "=");
		if (!ps) return NULL;
		
		cp_post_system_classpath(ps);	
		
        //grab an IVirtualMachine
        ivirtualmachine * ivm = new_virtualmachine();
		post("IVirtual Machine boot");
        //populate java options
        
        for(int i=0;i<numOptions;i++)
        {
            add_java_option(ivm, options[i].optionString);
        }
        
        //
        
		post("MXJClassloader CLASSPATH:");
		for(i = 0; i < props->len;i++) {
			char buff[1024];
			if (props->pptr[i]->id == MXJPROP_DYN_CLASS_DIR) {
				strcpy(buff, (char *)(props->pptr[i])->prop);
				post("   %s",buff);
                add_java_classpath(ivm, (char *)(props->pptr[i])->prop);
            }
		}
        
        start_java(ivm);
        g_jvm = get_java_vm(ivm);
        
        
//	    if ((err = MXJ_JNI_CREATE_JAVA_VM(&g_jvm, (void**)&env, &vmArgs)) != 0) {
        if(g_jvm==NULL){
	    		for(i = 0; i < numOptions;i++)
	    			sysmem_freeptr(options[i].optionString);		
	    	
	    	return NULL;
    	}
    
        env = get_thread_env(ivm);
	 
	 	ps_global_jvm->s_thing = (t_object *)g_jvm;
	    
	    for(i = 0; i < numOptions;i++)
	    	sysmem_freeptr(options[i].optionString);	
		mxj_inform_classloader();	
		
		return env;
    }
	
	return NULL;
}

long mxj_inlive(void)
{
	t_object *max = gensym("max")->s_thing;
	
	return class_findbyname(CLASS_NOBOX, gensym("livewind")) && !zgetfn(max,gensym("savefile"));
}

short mxj_generate_default_options(JavaVMOption* options)
{
	post("(mxj) max.java.config.txt is not present. Using default JVM options.");
	options[1].optionString = (char*)sysmem_newptr(32);
	options[1].extraInfo = NULL;
	options[2].optionString = (char*)sysmem_newptr(32);
	options[2].extraInfo = NULL;
	options[3].optionString = (char*)sysmem_newptr(32);
	options[3].extraInfo = NULL;
	
	sprintf(options[1].optionString,"-Xincgc");
	sprintf(options[2].optionString,"-Xms16m");
	sprintf(options[3].optionString,"-Xmx256m");
	
	return 4;
}

short mxj_get_jvmopts(JavaVMOption* options, int *num_options, int max_opts)
{
	short err;
	short path;
	t_fourcc outtype;
	t_fourcc typelist;
	void *b;
	long offset1;
	long offset2;
	t_atom at;
	t_string *classpath = NULL;
	int op_idx;	// current idx of other jvm options. classpath is always option 0
	char* prop_name;
	char  prop_val[MAX_PATH_CHARS];
	char configfile[MAX_PATH_CHARS];
	char tmp[MAX_PATH_CHARS];
	char path_to_maxjar[MAX_PATH_CHARS];
	char path_to_classes[MAX_PATH_CHARS];
	int i = 0;
	long len;
	char sep;
	
#ifdef WIN_VERSION		
	sep = '\\';
#else
	sep	= '/';
#endif	// WINVERSION
								
	outtype  = 0;
	typelist = 0;
	op_idx = 1;
	sprintf(configfile,JVM_CONFIG);
		
	sprintf(tmp,"max.jar");
	err = locatefile_extended(tmp, &path, &outtype, &typelist,0);
	if (err)
	{
		post("(mxj) Unable to find max.jar! mxj is rendered powerless in its absence."); 
		return MAX_ERR_GENERIC;
	}
	err = path_topathname(path,"",path_to_maxjar);
	if (err)
	{
		post("(mxj) Unable to resolve path of max.jar! mxj is rendered powerless.");
		return MAX_ERR_GENERIC;
	}

	// there is no point in setting up a default classpath: if we didn't find the max.jar, nothing loads
	// and if we do find the max.jar, those paths are overwritten.
	// plus those old paths are no longer relevant, disabling [jb]
	// string_append(classpath, CLASS_PATH);
	classpath = string_new("");

	max_path_to_native_path(path_to_maxjar,prop_val);
	cp_add_system_jar_dir(prop_val, path, classpath);
	len = (long)strlen(prop_val);
	if (prop_val[len-1] == sep)
		len--;
	for (i=len-1;i>=0&&prop_val[i]!=sep;i--) {
		;
	}
	prop_val[i+1] = '\0';

	sprintf(path_to_classes,"%sclasses%c",prop_val,sep);
	cp_add_dynamic_class_dir(path_to_classes);

	// new support for packages lib folder
	// TODO: how to handle projects? we actually should try to sandbox any loaded classes/jars
	// I'm not really sure if we can do that, though: in any case, we want to be able to dynamically
	// load and unload classes based on the availability of the project.
	// http://stackoverflow.com/questions/60764/how-should-i-load-jars-dynamically-at-runtime/1450837#1450837
	// might be something interesting.
	// In the short term, maybe we should just load any jars from the 'code' folder of a project and wait
	// until folks complain that there's no isolation.
	{
		t_linklist	*paths = packages_createsubpathlist("java-classes", false);
		t_ptr_size	pathcount = linklist_getsize(paths);
		t_ptr_size	i;

		for (i=0; i<pathcount; i++) {
			short	path = (short)(t_ptr_int)linklist_getindex(paths, i);
			short	jpath;
			char	maxpath[MAX_PATH_CHARS];
			char	fullpath[MAX_PATH_CHARS];

			// add any jars explicitly
			jpath = 0;
			if (!path_getpath(path, "lib", &jpath) && jpath) {
				if (!path_topathname(jpath, "", maxpath)) {
					max_path_to_native_path(maxpath, fullpath);
					cp_add_system_jar_dir(fullpath, jpath, classpath);
				}
			}
			jpath = 0;
			if (!path_getpath(path, "classes", &jpath) && jpath) {
				if (!path_topathname(jpath, "", maxpath)) {
					max_path_to_native_path(maxpath, fullpath);
					len = (long)strlen(fullpath);
					if (fullpath[len-1] != sep) {
						fullpath[len++] = sep;
						fullpath[len] = '\0';
					}
					cp_add_dynamic_class_dir(fullpath);
				}
			}
			// backward compatibility, load classes from the top-level of the java-classes folder
			// don't load jars, though.
			if (!path_topathname(path, "", maxpath)) {
				max_path_to_native_path(maxpath, fullpath);
				len = (long)strlen(fullpath);
				if (fullpath[len-1] != sep) {
					fullpath[len++] = sep;
					fullpath[len] = '\0';
				}
				cp_add_dynamic_class_dir(fullpath);
			}
		}
		object_free(paths);
	}

	//end set up of default...look for user additions in jvm.config
	err = locatefile_extended(configfile,&path,&outtype,&typelist,0);
	
	if (err) {	// couldn't find max.jvm.config..give some reasonable defaults
		op_idx = mxj_generate_default_options(options);
		goto make_classpath;	// cp is option[0]
	}
	
	b = binbuf_new();
	// this is deprecated, but since we are reading a deprecated file format
	// we will stick with it for now
	err = binbuf_read(b, JVM_CONFIG, path,0);
	
	if (err) {	// couldn't load jvm config
		binbuf_free(b);
		op_idx = mxj_generate_default_options(options);
		goto make_classpath;	// cp is option[0]
	}
	
	offset1 = offset2 = 0;
	while (!binbuf_getatom(b, &offset1, &offset2, &at))
	{
		if (at.a_type == A_SYM)
		{
			prop_name = at.a_w.w_sym->s_name;
			if (!strcmp(prop_name,PROP_CP_DIR))
			{
				char jpath[MAX_PATH_CHARS];
				binbuf_getatom(b,&offset1,&offset2,&at);
				max_path_to_native_path(at.a_w.w_sym->s_name,prop_val);
				if (*string_getptr(classpath)) {
					jpath[0] = CLASSPATH_SEPARATOR;
					jpath[1] = '\0';
				} else {
					jpath[0] = '\0';
				}
				strncat(jpath, prop_val, MAX_PATH_CHARS);
				mxj_string_appendtoclasspath(classpath, jpath);
			}
			else if (!strcmp(prop_name, PROP_DYNAMIC_CLASS_DIR))
			{
				binbuf_getatom(b,&offset1,&offset2,&at);
				max_path_to_native_path(at.a_w.w_sym->s_name,prop_val);
				cp_add_dynamic_class_dir(prop_val);
				
			}
			else if (!strcmp(prop_name,PROP_DYNAMIC_JAR_DIR))
			{
				char tmp[128];
						
				binbuf_getatom(b,&offset1,&offset2,&at);
				//we need to do this cpy since locate file extended will set filename to 0 since this is just a dir ref
				//and we want to preserve the dir ref so we can use it later when we prepend it to each jar found
				strcpy(tmp,at.a_w.w_sym->s_name);
				err = locatefile_extended(tmp,&path,&outtype,&typelist,0);
				if (!err)
				{
					void *fx;
					t_fourcc ftype;
					char fname[256];
					char *ps;
					char *dyn_cl_entry;
								
					fx = path_openfolder(path);
					max_path_to_native_path(at.a_w.w_sym->s_name,prop_val);
							
					while(path_foldernextfile(fx,&ftype,fname,false))
					{
						ps = &fname[0];		
						while(*ps++)
							;	
						if (*(ps - 2) ==  'r' &&
							*(ps - 3) == 'a' &&
							*(ps - 4) == 'j' &&
							*(ps - 5) == '.' )
							{	
								dyn_cl_entry = (char *)sysmem_newptr(384);
								dyn_cl_entry[0] = '\0';
										
								strcat(dyn_cl_entry,prop_val);
								if (dyn_cl_entry[strlen(dyn_cl_entry) - 1] != '/')
									strcat(dyn_cl_entry,"/");
								
								strcat(dyn_cl_entry,fname);
								//post("adding dyn cl entry %s",dyn_cl_entry);
								if (!mxj_proplist_find_stringprop(props, MXJPROP_DYN_CLASS_DIR, dyn_cl_entry)) {
									mxj_proplist_add_prop( props, MXJPROP_DYN_CLASS_DIR, (void *)dyn_cl_entry);
								} else {
									sysmem_freeptr(dyn_cl_entry);
								}
							}
					}
				}
			}
			else if (!strcmp(prop_name,PROP_JAR_DIR))
			{
				char tmp[MAX_PATH_CHARS];
						
				binbuf_getatom(b,&offset1,&offset2,&at);
				//we need to do this cpy since locate file extended will set filename to 0 since this is just a dir ref
				//and we want to preserve the dir ref so we can use it later when we prepend it to each jar found
				strcpy(tmp,at.a_w.w_sym->s_name);
				err = locatefile_extended(tmp,&path,&outtype,&typelist,0);
				if (!err)
				{
					max_path_to_native_path(at.a_w.w_sym->s_name,prop_val);
					cp_add_system_jar_dir(prop_val, path, classpath);
				}
			}
			else if (!strcmp(prop_name,PROP_JVM_OPT))
			{
				char *jvm_opt;
				
				binbuf_getatom(b,&offset1,&offset2,&at);
				jvm_opt = at.a_w.w_sym->s_name;
				if (strcmp(jvm_opt, "-Xdebug") || !mxj_inlive())
				{
					if (op_idx < max_opts)
					{
                        //need more than 64 byte options ber generous here
						options[op_idx].optionString = (char*)sysmem_newptr(256);
						options[op_idx].extraInfo = NULL;
						strcpy(options[op_idx].optionString,jvm_opt);
						op_idx++;	
					}
				}
			}
			else if (!strcmp(prop_name,PROP_DEBUG))//not implemented yet
				binbuf_getatom(b,&offset1,&offset2,&at);						
			else if (!strcmp(prop_name,PROP_CLASSES_FROM_DISK))
			{
				binbuf_getatom(b,&offset1,&offset2,&at);
				if (at.a_type == A_LONG)
				{
					if (at.a_w.w_long)
						mxj_proplist_add_prop(props,MXJPROP_LOAD_CLASS_FROM_DISK,(void *)1);
					else
						mxj_proplist_add_prop(props,MXJPROP_LOAD_CLASS_FROM_DISK,(void *)0);
				}
				else
					mxj_proplist_add_prop(props,MXJPROP_LOAD_CLASS_FROM_DISK,(void *)0);	
			}
			else if (!strcmp(prop_name,PROP_MXJ_MSP_MODE))
			{
				binbuf_getatom(b,&offset1,&offset2,&at);
				if (at.a_type == A_LONG)
				{
					t_atom_long mode = at.a_w.w_long;
					if (mode == MXJ_MSP_MODE_COMPATIBLE)
						mxj_proplist_add_prop(props,MXJPROP_MSP_MODE,(void *)MXJ_MSP_MODE_COMPATIBLE);
					else if (mode == MXJ_MSP_MODE_HIPERFORMANCE)
						mxj_proplist_add_prop(props,MXJPROP_MSP_MODE,(void *)MXJ_MSP_MODE_HIPERFORMANCE);			
					else//default to hi performance mode for now....
						mxj_proplist_add_prop(props,MXJPROP_MSP_MODE,(void *)MXJ_MSP_MODE_HIPERFORMANCE);		
				}

			}
			else if (!strcmp(prop_name,PROP_JAVA_JVM_VERSION))
			{
				binbuf_getatom(b,&offset1,&offset2,&at);
				if (at.a_type == A_FLOAT)
				{
					sprintf (g_java_jvm_version,"%3.1f", at.a_w.w_float);

				}
				if (at.a_type == A_SYM)
				{
					sprintf (g_java_jvm_version,"%s", at.a_w.w_sym->s_name);
				}
			
			}
			else
			{
				error("(mxj) unknown option %s in %s. ",prop_name,JVM_CONFIG);
				binbuf_free(b);
				goto make_classpath;
			}
		}
		else if (at.a_type ==  A_SEMI)
		{//parse up till next semicolon
			while(!binbuf_getatom(b,&offset1,&offset2,&at))
			{
				if (at.a_type == A_SEMI)
					break;
			}
		}
	}	
			
	binbuf_free(b);
	
make_classpath:

	options[0].optionString = (char*)sysmem_newptr(strlen(string_getptr(classpath)) + 256);
	options[0].extraInfo = NULL;
	sprintf(options[0].optionString,"-Djava.class.path=%s",string_getptr(classpath));
	*num_options = op_idx;

	object_free(classpath);

	return MAX_ERR_NONE;
}

void mxj_string_appendtoclasspath(t_string *classpath, char *jarpath)
{
	int offset = (*jarpath == CLASSPATH_SEPARATOR) ? 1 : 0;
	if (!(strstr(string_getptr(classpath), jarpath+offset))) {
		string_append(classpath, jarpath);
	}
}

void cp_add_system_jar_dir(char *native_dirname, short path, t_string *classpath)
{
	void *fx;
	t_fourcc ftype;
	char fname[128];
	char sep;
	char jarpath[MAX_PATH_CHARS];
	int idx = 0;

#ifdef WIN_VERSION
	sep = '\\';
#else
	sep	= '/';
#endif	// WINVERSION
								
	fx = path_openfolder(path);
	while(path_foldernextfile(fx, &ftype, fname, false)) {
		size_t len = strlen(fname);
		if (len >= 4
			&& !strncmp(fname + len - 4, ".jar", 4))
		{
			size_t jarlen;

			if (*string_getptr(classpath)) {
				jarpath[0] = CLASSPATH_SEPARATOR;
				jarpath[1] = '\0';
				idx = 1;
			} else {
				jarpath[0] = '\0';
			}
			strncat(jarpath, native_dirname, MAX_PATH_CHARS);
			jarlen = strlen(jarpath);
			if (jarpath[jarlen - 1] != sep) {
				jarpath[jarlen++] = sep;
				jarpath[jarlen] = '\0';
			}
			strncat(&jarpath[jarlen], fname, MAX_PATH_CHARS);
			mxj_string_appendtoclasspath(classpath, jarpath);
		}
	}
}

void cp_add_dynamic_class_dir(char* native_dirname)
{
	char* dyn_cl_entry;

	if (!mxj_proplist_find_stringprop(props, MXJPROP_DYN_CLASS_DIR, native_dirname)) {
		dyn_cl_entry = (char *)sysmem_newptr((long)strlen(native_dirname) + 2);
		strcpy(dyn_cl_entry, native_dirname);
		mxj_proplist_add_prop(props, MXJPROP_DYN_CLASS_DIR, (void *)dyn_cl_entry);
	}
}

void cp_post_system_classpath(char *sys_classpath)
{
    //classpaths can get much larger than 1024 be very generous here as well
    //too small a buffer is the cause of crashes
	char cp_out[10240];
	int idx;
	char c;
	char cp_sep;
	idx = 0;
	
	post("MXJ System CLASSPATH:");
	while(*sys_classpath++)
	{
		c = *sys_classpath;
#ifdef WIN_VERSION
		cp_sep = ';';
#else
		cp_sep = ':';
#endif

		if (c == cp_sep) {
			cp_out[idx] = '\0';
			post("   %s",cp_out);
			idx = 0;
		}
		else
			cp_out[idx++] = c;		
	}
	// get last one or only one if there is one entry
	cp_out[idx] = '\0';
	post("   %s", cp_out);
}

void jvm_release()
{
	post("(jvm_release)deallocating jvm");
	
	if (g_jvm != NULL) {
	   // free the Java virtual machine
#ifdef MXJ_FREE_VM
	   // seems not to work on OS X
	   MXJ_INVOKE_CALL(g_jvm,DestroyJavaVM)(g_jvm);
#endif
	   g_jvm = NULL;
	}
}

void setPeer(JNIEnv *env, t_maxjava *x)
{
	jfieldID mPeer_FID;
	
	mPeer_FID = MXJ_JNI_CALL(env,GetFieldID)(env, x->cls, PEER_FNAME, PEER_SIG);
	MXJ_JNI_CALL(env,SetLongField)(env, x->javaInstance, mPeer_FID, (t_atom_long)x);
}

void unsetPeer(JNIEnv *env, t_maxjava *x)
{
	jfieldID mPeer_FID;

	mPeer_FID = MXJ_JNI_CALL(env,GetFieldID)(env, x->cls, PEER_FNAME, PEER_SIG);
	MXJ_JNI_CALL(env,SetLongField)(env, x->javaInstance, mPeer_FID, 0L);
}

void setName(JNIEnv *env, t_maxjava *x, char* name)
{
	jfieldID mName_FID;
	jstring utfname;
	
	mName_FID = MXJ_JNI_CALL(env,GetFieldID)(env, x->cls, "mName", "Ljava/lang/String;");
	utfname = MXJ_JNI_CALL(env,NewStringUTF)(env,name);
	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, mName_FID,(jobject)utfname);
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,utfname);
}


void mxj_add_attr(t_maxjava* x, t_mxj_attr* a)
{
	if (x)
	{
		if (x->attr_num == 0) {	// first one
			x->attr_num++;
			x->attrlist = (t_mxj_attr **)sysmem_newptr(sizeof(t_mxj_attr *));
			*(x->attrlist) = a;
		}
		else {
			x->attr_num++;
			x->attrlist = (t_mxj_attr **)sysmem_resizeptr(x->attrlist, x->attr_num * sizeof(t_mxj_attr *));
			*(x->attrlist + (x->attr_num - 1)) = a;
		}
	}
}

void mxj_make_inlets(JNIEnv *env, t_maxjava *x)
{
	jint *inlet_types;
	jarray ja;
	jint array_len;
	jboolean is_copy;
	int i;
	jmethodID getInletsMID = 0;
	int num_proxies;
	int noinlet = 0;	
#ifdef MXJ_MSP
	int num_signal_inlets = 0;	
#endif

	getInletsMID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, "getInlets", "()[I");
	checkException(env);	
	ja =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, getInletsMID);
 	checkException(env);
	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
	inlet_types = MXJ_JNI_CALL(env, GetIntArrayElements)(env,(jintArray)ja,&is_copy);
	checkException(env);

	if (array_len == 0) {
		x->m_nInlets = 0;
#ifdef MXJ_MSP
		x->num_sig_inlets = num_signal_inlets;
		dsp_setup((t_pxobject *)x,x->num_sig_inlets);
#else	
		class_noinlet(ob_messlist(s_maxjava_class));
#endif
		return;	
	}

	noinlet = ob_class(x)->c_noinlet;

#ifdef MXJ_MSP
	for (i = array_len - 1; i >= 0; i--)  {
		if (inlet_types[i] == I_TYPE_SIGNAL) {
			array_len--;//"remove" signal inlets since they need to be created seperately
			num_signal_inlets++;
		}				
	}			

#endif//MXJ_MSP

	// We want nInlets total inlets, which means creating that number of proxies.
	// (We told the system not to give us a default inlet - normally we'd
	// only create nInlets - 1 proxies.)
	// Create them in reverse order (right to left)
	num_proxies = array_len - 1; // one is always created by default if we got to here

	//if a previous incarnation of mxj has noinlets we need to make sure that we make
	//the default one ourselves from now on.
	if (noinlet) num_proxies++;

#ifdef MXJ_MSP
	if (num_signal_inlets > 0)	// MSP is using the first proxy inlet so we need to make sure
		num_proxies++;			// we account for that
#endif
	
	if (num_proxies > 0)
		x->m_proxy = (void **)mxj_getbytes(num_proxies * sizeof(void *));
	else
		x->m_proxy = NULL;

	for (i = num_proxies - 1; i >= 0; i--) {	// create right to left
#ifdef MXJ_MSP
		x->m_proxy[i] = proxy_new(x, i + num_signal_inlets , &x->m_inletNumber);
#else
		if (noinlet)
			x->m_proxy[i] = proxy_new(x, i, &x->m_inletNumber);
		else
			x->m_proxy[i] = proxy_new(x, i + 1, &x->m_inletNumber);	// take default proxy inlet into account
#endif
	}	

#ifdef MXJ_MSP
	x->num_sig_inlets = num_signal_inlets;	
	dsp_setup((t_pxobject *)x,x->num_sig_inlets);
#endif
	
	x->m_nInlets = num_proxies;//MSP will free the one it creates by default
	
	if (is_copy) {
		MXJ_JNI_CALL(env, ReleaseIntArrayElements)(env,(jintArray)ja,inlet_types,0);
		checkException(env);
	}
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);

	return;
}


void mxj_make_outlets(JNIEnv *env, t_maxjava *x)
{
	jint *outlet_types;
	jarray ja;
	jint array_len;
	jboolean is_copy, create_info;
	int i;
	jmethodID getOutletsMID = 0;
	jfieldID mCreateInfoOutletFID = 0;
	
#ifdef MXJ_MSP
	int num_signal_outlets = 0;		
#endif

	getOutletsMID = MXJ_JNI_CALL(env,GetMethodID)(env, x->cls, "getOutlets", "()[I");
	checkException(env);
	mCreateInfoOutletFID = MXJ_JNI_CALL(env,GetFieldID)(env, x->cls, "mCreateInfoOutlet", "Z");
	checkException(env);
		
	ja =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, getOutletsMID);
 	checkException(env);
	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
	outlet_types = MXJ_JNI_CALL(env, GetIntArrayElements)(env,(jintArray)ja,&is_copy);
	checkException(env);
	create_info =  MXJ_JNI_CALL(env,GetBooleanField)(env, x->javaInstance, mCreateInfoOutletFID);
	checkException(env);
	
	//don't forget to free this m_outlet pointer. we don't need to free outlets themselves. max does this for us.
	if (create_info) {
		x->m_outlet = (void **)mxj_getbytes((array_len + 1) * sizeof(void *));
		x->m_outlet[array_len] = outlet_new(x,0L);	// nfo outlet is the rightmost
		x->nfo_outlet = x->m_outlet[array_len];
		x->m_nOutlets = array_len + 1;
	} else {
		x->m_outlet = (void **)mxj_getbytes(array_len * sizeof(void *));
		x->nfo_outlet = 0;
		x->m_nOutlets = array_len;
	}	
		
	for (i = array_len - 1; i >= 0; i--) {
		void *o = NULL;
		switch(outlet_types[i]) {
			case O_TYPE_BANG:
				o = bangout(x);
				break;
			case O_TYPE_INT:
				o = intout(x);
				break;
			case O_TYPE_FLOAT:
				o = intout(x);
				break;
			case O_TYPE_LIST:
				o = listout(x);
				break;
			case O_TYPE_MESS:		
			case (O_TYPE_INT | O_TYPE_FLOAT | O_TYPE_LIST | O_TYPE_MESS):
				o = outlet_new(x, 0L);
				break;
	#ifdef MXJ_MSP
			case O_TYPE_SIGNAL:
				num_signal_outlets++;
				break;
	#endif	
			default:
				error("(mxj) unknown outlet type %d. Defaulting to anything outlet.",outlet_types[i]);
				o = outlet_new(x, 0L);
		}
		
		x->m_outlet[i] = o;
	}
	
#ifdef MXJ_MSP
	x->num_sig_outlets = num_signal_outlets;
	for(i = 0; i < num_signal_outlets;i++)
		outlet_new((t_object *)x,"signal");
#endif
	
	if (is_copy) {
		MXJ_JNI_CALL(env, ReleaseIntArrayElements)(env,(jintArray)ja,outlet_types,0);
		checkException(env);
	}

	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
	
	return;
}

///@ exec units ////////////////////////////////////////////////////////////////////////////////////////
void at_exec_unit_print(t_at_exec_unit *x)
{
	int i;
	
	post("AT EXEC UNIT");
	post("------------");
	post("name: %s",x->msg->s_name);
	post("argc %d",x->argc);
	
	for(i = 0; i < x->argc;i++)
	{
		switch(x->argv[i].a_type)
		{
			case A_SYM:
				post("arg %d sym: %s",i,x->argv[i].a_w.w_sym->s_name);
				break;
			case A_LONG:
				post("arg %d long: %d",i,x->argv[i].a_w.w_long);
				break;
			case A_FLOAT:
				post("arg %d float: %f",i,x->argv[i].a_w.w_float);
				break;
		}
	}
post(" ");
}


t_at_exec_unit *at_exec_unit_new(t_symbol *msg)
{
	t_at_exec_unit *x;

	x = (t_at_exec_unit*) sysmem_newptr(sizeof(t_at_exec_unit));
	x->msg = msg;
	x->argc = 0;
	x->argv = 0; 
	
	//post("creating new @ exec unit: %s",msg->s_name);
	return x;
}

void at_exec_unit_push(t_at_exec_unit *x, t_atom *a)
{
	if (x->argv == 0)
		x->argv = (t_atom*)sysmem_newptr(sizeof(t_atom));
	else
		x->argv = (t_atom *)sysmem_resizeptr(x->argv,(x->argc + 1) * sizeof(t_atom));	
	sysmem_copyptr(a,x->argv + x->argc,sizeof(t_atom));
	x->argc++;
}

void at_exec_unit_free(t_at_exec_unit* x)
{
	sysmem_freeptr(x->argv);
	sysmem_freeptr(x);
}

t_at_exec_unit *get_exec_unit(int *counter,short argc, t_atom *argv)
{
	t_symbol *msg;
	t_at_exec_unit *aeu;
	int i;
	t_atom a;
	
	a = argv[*counter];
	//get rid of at
	msg  = gensym((a.a_w.w_sym->s_name) + 1);
	aeu = at_exec_unit_new(msg);
	for(i = ((*counter) + 1); i < argc;i++) {
		//ran into another @ arg
		if (argv[i].a_type == A_SYM && argv[i].a_w.w_sym->s_name[0] == '@') {
			i--; // we need to rewind here so that the enclosing for loop in the caller
			     // will hit this sym and call us again. counter is callers for loop counter
			break;	
		}
		else
			at_exec_unit_push(aeu,&argv[i]);		
	}
	
	*counter = i;
	
	return aeu;
}

void mxj_add_at_exec_unit(t_maxjava *x,t_at_exec_unit *aeu)
{
	if (x->aeu_list == 0)
		x->aeu_list = (t_at_exec_unit **)sysmem_newptr(sizeof(t_at_exec_unit *));
	else
		x->aeu_list = (t_at_exec_unit **)sysmem_resizeptr(x->aeu_list,(x->aeu_num + 1)*sizeof(t_at_exec_unit *));

	x->aeu_list[x->aeu_num] = aeu;
	x->aeu_num++;		
}

void parse_out_at_args(t_maxjava *x, short *argc,t_atom *argv)
{
	int i;
	int first_at;


	first_at = -1;	
	for(i = 0;i < *argc;i++)
	{
		t_atom a;
		a = argv[i];
		if (a.a_type == A_SYM && a.a_w.w_sym->s_name[0] == '@')
		{
			t_at_exec_unit *aeu;
			if (first_at == -1)
				first_at = i;
				
			aeu = get_exec_unit(&i,*argc,argv);
			mxj_add_at_exec_unit(x,aeu);
		
		}	
	}
			
	if (first_at != -1)
		*argc = first_at;
}

void mxj_at_exec_units_exec(t_maxjava *x, int low_priority)
{
	int i;
	t_at_exec_unit *aeu;
	
	for(i = 0; i < x->aeu_num;i++)
	{
		aeu = x->aeu_list[i];
		
		if (mxj_get_attr(x,aeu->msg))
		{
			if (low_priority)
				defer_low(x,(method)maxjava_anything,aeu->msg,aeu->argc,aeu->argv);
			else
				maxjava_anything(x,aeu->msg,aeu->argc,aeu->argv);
		}
		else
			error("(mxj) @%s does not resolve to a declared attribute. Ignoring.",aeu->msg->s_name);
	}
}



#ifdef MAC_VERSION
//
// Initialize AWT/////////////////////////////////////////////////////////////
//
void source_callback(void *info)
{
	post("Called source callback");
}

/*
 * Find and load the named library/bundle.
 */
CFBundleRef getMachOLibrary(CFStringRef bundleName) {
	//	Make a CFURLRef from the CFString representation of the bundle's path.
	//	See the Core Foundation URL Services chapter for details.
	CFURLRef bundleURL =
	CFURLCreateWithFileSystemPath(NULL, bundleName, kCFURLPOSIXPathStyle, true);
	if (bundleURL) {
		Boolean didLoad;
		// Make a bundle instance using the URLRef.
		CFBundleRef bundle = CFBundleCreate(NULL, bundleURL);
		if (bundle) {
			didLoad = CFBundleLoadExecutable(bundle);
			if (didLoad) {
				CFRelease(bundleURL);
				return bundle;
			}
			
			CFRelease(bundle);
		}
		
		CFRelease(bundleURL);
		return NULL;
	}
	
	return NULL;
}

short init_awt()
{
	CFBundleRef bundle=NULL;
	t_mpV pmo_wk_init = 0;
	int res = 0;
			
	if ((bundle = getMachOLibrary(CFSTR("/System/Library/Frameworks/AppKit.framework")))) {
    	pmo_wk_init = (t_mpV) CFBundleGetFunctionPointerForName(bundle, CFSTR("NSApplicationLoad"));
   		res = (int)(t_atom_long)pmo_wk_init();//this returns 1 on success and 0 in failure

    	if (!res) {
    		error("(mxj) unable to call NSApplication load in awt_init");
    		return -1;
    	}
    }
	else {
		error("(mxj) unable to find AppKit bundle in awt_init");
		return -1;
	}

	systhread_create((method)awt_init_func,NULL,0,0,0,&s_awt_init_thread);	
	return 0;
}

void awt_init_func()
{
	JNIEnv *env;
	jclass clazz;
	int err;

	THREADENV(env);
	
	clazz = MXJ_JNI_CALL(env,FindClass)(env,"java/awt/Frame");	
	if (clazz == NULL) {
		error("(mxj)unable to initialize java/awt/Frame. AWT is unavailable.");
	}

	//Pass control back to main application.
	PMO_CFRunLoopStop(rl);
	
	//this will free clazz local ref
	err = MXJ_INVOKE_CALL(g_jvm,DetachCurrentThread)(g_jvm);	
	if (err) {
		error("(mxj) unable to detach awt init thread from jvm");
	}
	
	systhread_exit(0);
}

#endif

void mxj_inform_classloader()
{
	int i;
	JNIEnv * env;
	THREADENV(env);
	
	if (props != NULL)
	{	
		for(i = 0; i < props->len;i++)
		{
			if (props->pptr[i]->id == MXJPROP_DYN_CLASS_DIR)	// handles dyn jars as well
				mxj_classloader_add_dir(env,(char *)(props->pptr[i])->prop);
		}
	}
	
}
	
void* mxj_get_prop_val(int prop_id, void* default_val)
{
	int i;
	for(i = 0; i < props->len;i++) {
		if (props->pptr[i]->id == prop_id)//handles dyn jars as well
			return props->pptr[i]->prop;
	}
	
	return (void*)default_val;
}


////////////////////////////FREE D3WD/////////////////////////////////////

/*
 * Free this instance.
 */
void maxjava_free(t_maxjava *x) {
	short i;
	JNIEnv *env;
	if (g_jvm)
	{
		THREADENV(env);

		#ifdef MXJ_MSP
			//This has to happen first. Please don't move otherwise perform will be called while we are freeing
			//resources used in perform-toph
			dsp_free((t_pxobject *)x);	
		#endif
		
			
		if (x->javaInstance != NULL) 
		{
			mxjObjectWasDeleted(env, x);
			unsetPeer(env, x);
			MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->javaInstance);
			x->javaInstance = NULL;
		}

	    if (x->cls != NULL)
	    {
			MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->cls);
			x->cls = NULL;
		}
		
	mxj_freebytes(x->m_outlet,(x->m_nOutlets) * sizeof(void *));	
	mxj_freebytes(x->args,(x->argc) * sizeof(t_atom));
	//we know this is the length since we allocated it based on the length
	//strlen does not return terminating NULL
	
	//this could be a bug here for msp objects regarding how many proxies are freed
	mxj_freebytes(x->p_classname ,(long)(strlen(x->p_classname) + 1));
	if (x->m_proxy) {
		for (i = 0; i < x->m_nInlets ; i++) 
			freeobject(x->m_proxy[i]);
		if (x->m_nInlets)
			 mxj_freebytes(x->m_proxy,x->m_nInlets * sizeof(void *));		
	}	
	for(i = 0; i < x->attr_num; i++)
		mxj_attr_free(x->attrlist[i]);	
	for(i = 0; i < x->aeu_num;i++)
		at_exec_unit_free(x->aeu_list[i]);
	
	mxj_methodlist_free(x->methodlist);

#ifdef MXJ_MSP
	//Do Clean up for dsp method calls...
	if (x->dsp_call_cnt)//dsp was called
	{
		if (mxj_get_prop_val(MXJPROP_MSP_MODE,(void*)MXJ_MSP_MODE_HIPERFORMANCE) == (void*)MXJ_MSP_MODE_COMPATIBLE ||
		   mxj_get_prop_val(MXJPROP_MSP_MODE,(void*)MXJ_MSP_MODE_HIPERFORMANCE) == (void*)MXJ_MSP_MODE_HIPERFORMANCE)
		{//MXJ MSP COMPATIBLE MODE
			//this is just an empty MSPSignal[] array if there are 0 sig inlets
			MXJ_JNI_CALL(env,DeleteGlobalRef)(env,x->msp_sig_inlets);
			if (x->num_sig_inlets > 0)
			{

				for(i = 0; i < x->num_sig_inlets;i++)
				{
					MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->inlet_msp_sigs[i]);
					MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->inlet_msp_vecs[i]);			
				}
		
				mxj_freebytes(x->inlet_msp_sigs,(x->num_sig_inlets)* sizeof(jobject));
				mxj_freebytes(x->inlet_msp_vecs,(x->num_sig_inlets)* sizeof(jfloatArray));
			}
		//this is just an empty MSPSignal[] array if there are 0 sig inlets
			MXJ_JNI_CALL(env,DeleteGlobalRef)(env,x->msp_sig_outlets);
			if (x->num_sig_outlets > 0)
			{

				if (x->dsp_call_cnt)
				{
					for(i = 0; i < x->num_sig_outlets;i++)
					{
						MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->outlet_msp_sigs[i]);
						MXJ_JNI_CALL(env,DeleteGlobalRef)(env, x->outlet_msp_vecs[i]);			
					}
				mxj_freebytes(x->outlet_msp_sigs,(x->num_sig_outlets)* sizeof(jobject));
				mxj_freebytes(x->outlet_msp_vecs,(x->num_sig_outlets)* sizeof(jfloatArray));	
				}
			}
		}
	}//end if x->dsp_call_cnt
#endif
 }//end if g_jvm
}


void init_mxj_jitter(JNIEnv *env)
{
	t_object *x;
	t_symbol *j=gensym("jitter");
	char filename[MAX_PATH_CHARS];
	short vol;
	t_fourcc type;
	
	sprintf(filename,"jitter.jar");
	
	if (locatefile_extended(filename,&vol,&type,0L,0))
		return;

	if (!j->s_thing) {
		t_fourcc typelist[TYPELIST_SIZE];
		short numtypes;
		
		typelist_make(typelist,TYPELIST_EXTERNS,&numtypes);
		
		// if jit.matrix isn't present fail quietly
		sprintf(filename,"jit.matrix");
		if (locatefile_extended(filename,&vol,&type,typelist,numtypes)) {
			return;
		}
		
		x = newinstance(gensym("jit.matrix"),0,0);
		if (x)
			freeobject(x);
	}
	if (j->s_thing) {
		typedmess((t_object *)j->s_thing,gensym("javainit"),0,NULL);
	}
}
