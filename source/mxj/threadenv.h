#ifndef _Included_threadenv_h
#define _Included_threadenv_h

//#define THREADENV(env) (*g_jvm)->AttachCurrentThread(g_jvm, (void **)&env, NULL)
#define THREADENV(env) MXJ_INVOKE_CALL(g_jvm,AttachCurrentThread)(g_jvm,(void **)&env,NULL)

#endif
