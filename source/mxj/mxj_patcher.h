#ifndef __MXJ_PATCHER_H
#define __MXJ_PATCHER_H

#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif

void init_mxj_patcher(JNIEnv*);

#endif
