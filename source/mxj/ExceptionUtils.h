#ifndef _Included_ExceptionUtils_h
#define _Included_ExceptionUtils_h

void JNU_ThrowByName(JNIEnv *env, const char *name, const char *msg);

int checkException(JNIEnv *env);

#endif
