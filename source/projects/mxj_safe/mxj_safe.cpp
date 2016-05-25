// Copyright (c) 2016, Cycling '74
// Timothy Place
// Usage of this file and its contents is governed by the MIT License


#include "c74_msp.h"

using namespace c74::max;


#define DEFAULT_OS_ARCH "x86_64" // do we need to worry about 32-bit?
#include "../../mxj/JavaHomeOsx.c"
#include <dlfcn.h>
#include <string>


bool java_installed() {
	char* baseDir = getJavaHome();
	
	if (baseDir == nullptr || !strcmp(baseDir, "/"))
		return false;
	else
		return true;
}


static t_class *mxj_class = nullptr;
static t_class *mxj_tilde_class = nullptr;



struct mxj_safe {
	t_pxobject	base;
};


void mxj_safe_dopost(mxj_safe* self) {
	object_error_obtrusive((t_object*)self, "Java is not installed on this computer. Please install it from https://support.apple.com/kb/DL1572?locale=en_US.");
}


mxj_safe* mxj_safe_new(t_symbol* name, long ac, t_atom* av) {
	mxj_safe* self;
	
	if (name == gensym("mxj~"))
		self = (mxj_safe*)object_alloc(mxj_class);
	else
		self = (mxj_safe*)object_alloc(mxj_class);
	
	defer_low(self, (method)mxj_safe_dopost, nullptr, 0, nullptr);
	return self;
}


void ext_main(void* r) {
	if (java_installed()) {
		// object_post(nullptr, "java is installed... nothing to see... please move along...");
	}
	else {
		// object_error(nullptr, "java is not installed... defining dummies!");
		
		auto c = class_new("mxj", (method)mxj_safe_new, nullptr, sizeof(mxj_safe), nullptr, A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"int", A_LONG, 0);
		class_addmethod(c, (method)method_false,	"float", A_FLOAT, 0);
		class_addmethod(c, (method)method_false,	"bang",	0);
		class_addmethod(c, (method)method_false,	"list", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"anything", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"viewsource", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"zap", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"quickref",A_CANT,0);
		class_addmethod(c, (method)method_false,	"assist", A_CANT, 0);
		class_addmethod(c, (method)method_false,	"get", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"objectfilename", A_CANT, 0);
		class_addmethod(c, (method)method_false,	"save2", A_CANT, 0);
		class_addmethod(c, (method)method_false,	"loadbang",A_CANT,0);
		class_addmethod(c, (method)method_false,	"dblclick",A_CANT,0);
		class_addmethod(c, (method)method_false,	"fileusage", A_CANT, 0);
		class_register(CLASS_BOX, c);
		mxj_class = c;
		
		object_method_direct(void, (t_object*, t_symbol*, t_symbol*, t_symbol*),
							 gensym("max")->s_thing, gensym("objectfile"), gensym("mxj"), gensym("mxj_safe"), gensym("mxj"));

		

		c = class_new("mxj~", (method)mxj_safe_new, nullptr, sizeof(mxj_safe), nullptr, A_GIMME, 0);
		class_dspinit(c);
		class_addmethod(c, (method)method_false,	"int", A_LONG, 0);
		class_addmethod(c, (method)method_false,	"float", A_FLOAT, 0);
		class_addmethod(c, (method)method_false,	"bang",	0);
		class_addmethod(c, (method)method_false,	"list", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"anything", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"viewsource", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"zap", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"quickref",A_CANT,0);
		class_addmethod(c, (method)method_false,	"assist", A_CANT, 0);
		class_addmethod(c, (method)method_false,	"get", A_GIMME, 0);
		class_addmethod(c, (method)method_false,	"objectfilename", A_CANT, 0);
		class_addmethod(c, (method)method_false,	"save2", A_CANT, 0);
		class_addmethod(c, (method)method_false,	"loadbang",A_CANT,0);
		class_addmethod(c, (method)method_false,	"dblclick",A_CANT,0);
		class_addmethod(c, (method)method_false,	"fileusage", A_CANT, 0);
		//class_addmethod(c, (method)mxj_dsp64,						"dsp64", A_CANT, 0);
		//class_addmethod(c, (method)mxj_dspstate,					"dspstate", A_CANT, 0);
		//class_addmethod(c, (method)mxj_benchmark,					"benchmark",A_DEFLONG,A_DEFLONG,0);
		//class_addmethod(c, (method)mxj_exception_check,				"exceptioncheck",A_DEFLONG,0);
		class_register(CLASS_BOX, c);
		mxj_tilde_class = c;

		object_method_direct(void, (t_object*, t_symbol*, t_symbol*, t_symbol*),
							 gensym("max")->s_thing, gensym("objectfile"), gensym("mxj~"), gensym("mxj_safe"), gensym("mxj~"));
		
	
	}
}


