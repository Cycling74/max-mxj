#ifndef _Included_maxjava_h
#define _Included_maxjava_h

#include "mxj_common.h"
#include "mxj_methodlist.h"
#include "mxj_attr.h"
#include "mxj_utils.h"

#ifdef MXJ_MSP
	#include "z_dsp.h"
#endif

// The info for the peer field in MaxObject
#define PEER_FNAME	"mPeer"
#define PEER_SIG	"J"

// The info for the sStaticCallbacks field in MaxObject
#define CALLBACKS_TABLE_FNAME	"sCallbacks"
#define CALLBACKS_TABLE_SIG		"J"

// How many chars a post call should contain, maximum
#define ASSIST_MAX (60)

typedef enum {
	MXJ_ERR_NONE =			0,	///< No error
	MXJ_ERR_GENERIC =		-1,	///< Generic error
	MXJ_ERR_BAIL =			-2,	///< Client wanted to bail out
} t_mxj_err;

//This is used to defer handling of @ args
typedef struct at_exec_unit
{
	t_symbol *msg;
	short argc;
	t_atom *argv;
}t_at_exec_unit;

typedef struct _pxobject_dummy
{
	long  _dummy1;  //t_maxjava compatibility for mxj and mxj~..essentially t_pxobj
	void* _dummy2;
	long  _dummy3;
	short _dummy4;
	short _dummy5;
} t_pxobject_dummy;

typedef struct maxjava	// defines our object's internal variables for each instance in a patch
{
#ifdef MXJ_MSP
	t_pxobject p_ob;			
#else
	t_object p_ob;				// object header
	t_pxobject_dummy p_dummy;	// we need to define our struct like above to ensure byte compatibility with struct layout
#endif
	
	// inlets
	int		m_nInlets;      // will be 1 more than the number of proxies
	void	**m_proxy;
	long	m_inletNumber; // where proxy will put the inlet number

    // outlets
    int		m_nOutlets;		// number of user created outlets
    void	**m_outlet;		// the user outlets
    void	*nfo_outlet;	// mxj nfo outlet

    //max stuff
    t_patcher *thispatcher;	// the patcher we are associated with, set in maxjava_new
    t_box *thisbox;			// the box we are associated with, set in maxjava_new
    
    // Java stuff
	jobject	javaInstance;	// our corresponding Java object
	jclass	cls;			// the class of our corresponding Java object
	char	*p_classname;	// peer classname as provided by the user in the MaxBox
	
	jmethodID inlet_bang_MID;		// method ID for void bang()
	jmethodID inlet_int_MID;		// method ID for void inlet(int, int)
	jmethodID inlet_float_MID;		// method ID for void inlet(int, float)
	jmethodID inlet_anything_MID;	// method ID for void inlet(int, String, Atom[])

	jmethodID getInletAssist_MID;	// get assistance string for an inlet
	jmethodID getOutletAssist_MID;	// get assistance string for an outlet
	
	void	*binbuf;	// set when save is called
	short	argc;		// number of instantiation arguments
	t_atom	*args;		// instantiation arguments
	 
	t_mxj_methodlist	*methodlist;	// list of registered (mxj_methodlist_add) methods
	t_mxj_attr			**attrlist;		// list of registered (mxj_add_attr) attributes
	int					attr_num;		// attribute count
	t_at_exec_unit		**aeu_list;		// @ argument list, used during object construction
	int					aeu_num;		// number of @ arguments
	
#ifdef MXJ_MSP
	JNIEnv* aenv;				// audio thread java exec environment
	jmethodID sig_perform_MID;	// method id of current perform method
	jmethodID dsp_MID;			// method id of java dsp method
	jmethodID dspstate_MID;		// method id of java dsp method
	int num_sig_inlets;			// current number of active sig inlets
	int num_sig_outlets;		// current number of active signal outlets

	jobject* inlet_msp_sigs;		// MSPSignal inlet members
	jobject* outlet_msp_sigs;		// MSPSignal outlet members
	jfloatArray* inlet_msp_vecs;	// MSP signal input vec float[] objects
	jfloatArray* outlet_msp_vecs;	// MSP signal output vec float[] objects
	jobject  msp_sig_inlets;		// MSPSignal[] inlets for perform/dsp
	jobject  msp_sig_outlets;		// MSPSignal[] outlets for perform/dsp
	int dsp_call_cnt;				// num times dsp was called on this instance

	//benchmark perform stuff
	int dsp_running;
	int benchmark_perform;
	int benchmark_interval;
	int benchmark_loops;
	double benchmark_total;
	double benchmark_best_time;
	double benchmark_worst_time;
	
	//perf exception check
	int perf_exception_occurred;
	int perf_exception_check;		
#endif
} t_maxjava;

/**
 register an attribute
 
 @param 	x	the maxjava object
 @param 	a	the attribute of type t_mxj_attr to register
*/
void mxj_add_attr(t_maxjava* x, t_mxj_attr* a);

/**
 get a registered attribute
 
 @param 	x	the maxjava object
 @param 	a	the name of the attribute as given in t_mxj_attr
*/
t_mxj_attr* mxj_get_attr(t_maxjava *x, t_symbol *name);

/**
 call a java method trying to coerce argument list to match the types
 given in sig (see t_mxj_method->jp_types)
 
 @param 	x		the maxjava object
 @param 	mid		method ID
 @param		sig		java siganture
 @param 	argc	The count of arguments in argv
 @param 	argv	Array of t_atoms; the method arguments
*/
void call_method_with_coercion(t_maxjava *x, jmethodID mid, t_symbol *sig, short argc, t_atom *argv);

/**
 coerce max atom into symbol
 
 @param 	idx		the index of the atom to coerce
 @param 	argc	The count of arguments in argv
 @param 	argv	Array of t_atoms; a argument list
*/
t_symbol *atomargs_getsym(long idx, short ac, t_atom *av);

/**
 coerce max atom into java long
 
 @param 	idx		the index of the atom to coerce
 @param 	argc	The count of arguments in argv
 @param 	argv	Array of t_atoms; a argument list
*/
jlong atomargs_getlong(long idx, short ac, t_atom *av);

/**
 coerce max atom into java float
 
 @param 	idx		the index of the atom to coerce
 @param 	argc	The count of arguments in argv
 @param 	argv	Array of t_atoms; a argument list
*/
jdouble atomargs_getfloat(long idx, short ac, t_atom *av);

/**
 coerce max atom into java char
 
 @param 	idx		the index of the atom to coerce
 @param 	argc	The count of arguments in argv
 @param 	argv	Array of t_atoms; a argument list
*/
jchar atomargs_getchar(long idx, short ac, t_atom *av);

/**
 coerce max atom into java bool
 
 @param 	idx		the index of the atom to coerce
 @param 	argc	The count of arguments in argv
 @param 	argv	Array of t_atoms; a argument list
*/
jboolean atomargs_getboolean(long idx, short ac, t_atom *av);

#endif
