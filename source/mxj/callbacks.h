#ifndef _Included_callbacks_h
#define _Included_callbacks_h

void init_max_object_callbacks(JNIEnv*);
short i_am_a_max_thread(void);      //   i am the low or high priority thread but not the java thread
short i_am_in_java_constructor(void);

typedef void (*t_outputfunc)(t_object *, C74_CONST char *, ...);

void say(t_outputfunc outputFn,
		 JNIEnv *env,
		 jclass cls,
		 jbyteArray bytes);

#endif
