#ifndef _MXJ_MAX_SYS_H
#define _MXJ_MAX_SYS_H

#ifdef MAC_VERSION
#include <JavaVM/jni.h>        // Java Native Interface definitions
#include <JavaVM/jni_md.h>
#else
#include "jni.h"
#include "jni_md.h"
#endif

void init_mxj_max_system(JNIEnv*);

#endif
