#ifndef _Included_clock_callbacks_h
#define _Included_clock_callbacks_h

#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif

void init_clock_callbacks(JNIEnv*);

#endif
