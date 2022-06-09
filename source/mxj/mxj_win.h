

#ifndef _MXJ_WIN_H_
#define _MXJ_WIN_H_

#include <windows.h>
#include "jni.h"

/***********************************************************
 * from launcher/java.h 
 ***********************************************************/

/*
 * Pointers to the needed JNI invocation API, initialized by LoadJavaVM.
 */
typedef jint (JNICALL *CreateJavaVM_t)(JavaVM **pvm, void **env, void *args);
typedef jint (JNICALL *GetDefaultJavaVMInitArgs_t)(void *args);

typedef struct {
    CreateJavaVM_t CreateJavaVM;
    GetDefaultJavaVMInitArgs_t GetDefaultJavaVMInitArgs;
} InvocationFunctions;

/*
 * Protoypes for launcher functions in the system specific java_md.c.
 */

jboolean
LoadJavaVM(const char *jvmpath, InvocationFunctions *ifn);

void
GetXUsagePath(char *buf, jint bufsize);

jboolean
GetApplicationHome(char *buf, jint bufsize);

const char *
GetArch();

long CreateExecutionEnvironment(
					   char jrepath[],
				       jint so_jrepath,
				       char jvmpath[],
				       jint so_jvmpath);

/*
 * Report an error message to stderr or a window as appropriate.  The
 * flag always is set to JNI_TRUE if message is to be reported to both
 * strerr and windows and set to JNI_FALSE if the message should only
 * be sent to a window.
 */
void ReportErrorMessage(char * message, jboolean always);
void ReportErrorMessage2(char * format, char * string, jboolean always);

jboolean RemovableMachineDependentOption(char * option);
void PrintMachineDependentOptions();

/* 
 * Functions defined in java.c and used in java_md.c.
 */
jint ReadKnownVMs(const char *jrepath); 

/***********************************************************
 * from launcher/java_md.h 
 ***********************************************************/


#define PATH_SEPARATOR	';'
#define FILE_SEPARATOR	'\\'
#define MAXPATHLEN      MAX_PATH_CHARS

#ifdef JAVA_ARGS
/*
 * ApplicationHome is prepended to each of these entries; the resulting
 * strings are concatenated (seperated by PATH_SEPARATOR) and used as the
 * value of -cp option to the launcher.
 */
#ifndef APP_CLASSPATH
#define APP_CLASSPATH        { "\\lib\\tools.jar", "\\classes" }
#endif
#endif

/*
 * Support for doing cheap, accurate interval timing.
 */
extern jlong CounterGet(void);
extern jlong Counter2Micros(jlong counts);

long mxj_platform_init(void);

#endif /*_MXJ_WIN_H_*/
