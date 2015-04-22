#ifndef _Included_classpath_h
#define _Included_classpath_h

// Where Java stuff lives
#ifdef MAC_VERSION
#define JAVA_DIR "/Library/Application Support/Cycling '74/java"
#define JAVA_DIR_PRE JAVA_DIR"/"
#endif
#ifdef WIN_VERSION //should double check if we need drive letter, and if so make sure we're using the right letter
#define JAVA_DIR "c:\\Program Files\\Common Files\\Cycling '74\\java"
#define JAVA_DIR_PRE JAVA_DIR"\\"
#endif

// Where system classes, the policy file, and the JNI library go
#define SYSTEM_CLASSES JAVA_DIR_PRE"lib"
#ifdef MAC_VERSION
#define SYSTEM_CLASSES_PRE SYSTEM_CLASSES"/"
#endif
#ifdef WIN_VERSION
#define SYSTEM_CLASSES_PRE SYSTEM_CLASSES"\\"
#endif

#ifdef MAC_VERSION
#define CLASSPATH_SEPARATOR ':'
#endif
#ifdef WIN_VERSION
#define CLASSPATH_SEPARATOR ';'
#endif

// Where user classes go
#define USER_CLASSES JAVA_DIR_PRE"classes"

// The full classpath: max.jar, examples.jar, tests.jar, and the user's classes
#ifdef MAC_VERSION
#define CLASS_PATH SYSTEM_CLASSES_PRE"max.jar:"SYSTEM_CLASSES_PRE"examples.jar:"SYSTEM_CLASSES_PRE"tests.jar:"USER_CLASSES
#endif
#ifdef WIN_VERSION
#define CLASS_PATH SYSTEM_CLASSES_PRE"max.jar;"SYSTEM_CLASSES_PRE"examples.jar;"SYSTEM_CLASSES_PRE"tests.jar;"USER_CLASSES
#endif

#endif
