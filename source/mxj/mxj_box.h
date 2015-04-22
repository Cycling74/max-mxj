#ifndef __MXJ_BOX_H

#define _MXJ_BOX_H

#ifdef MAC_VERSION
#include <JavaVM/jni.h>        // Java Native Interface definitions
#include <JavaVM/jni_md.h>
#else
#include "jni.h"
#include "jni_md.h"
#endif

void init_mxj_box(JNIEnv*);

#endif