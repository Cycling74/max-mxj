/*
 * Information related to Java Atoms.
 */

#include "mxj_common.h"
#include "package.h"

#ifndef _Included_atom_h
#define _Included_atom_h

#define MAX_ATOM_CLASSNAME MAX_FRAMEWORK_PKG"/Atom"

#define NEW_INT_ATOM_MNAME "newAtom"
#define NEW_INT_ATOM_SIG "(I)L"MAX_ATOM_CLASSNAME";"

#define NEW_FLOAT_ATOM_MNAME "newAtom"
#define NEW_FLOAT_ATOM_SIG "(F)L"MAX_ATOM_CLASSNAME";"

#define NEW_STRING_ATOM_MNAME "newAtom"
#define NEW_STRING_ATOM_SIG "(L"STRING_CLASSNAME";)L"MAX_ATOM_CLASSNAME";"

#define GET_INT_VALUE_MNAME "getInt"
#define GET_INT_VALUE_SIG "()I"
#define GET_FLOAT_VALUE_MNAME "getFloat"
#define GET_FLOAT_VALUE_SIG "()F"
#define GET_STRING_VALUE_MNAME "getString"
#define GET_STRING_VALUE_SIG "()L"STRING_CLASSNAME";"

// These must be kept in sync with the definitions in Atom.java and AtomDataTypes.java.

// field mType is an int.
#define GET_ATOM_TYPE_MNAME "getType"
#define GET_ATOM_TYPE_SIG "()I"

// These are defined as static final ints in AtomDataTypes.
#define ATOM_TYPENUM_LONG    1
#define ATOM_TYPENUM_FLOAT   2
#define ATOM_TYPENUM_STRING  3

/*
 * Create a new Atom array and return it.
 */
jobjectArray newAtomArray(JNIEnv *env, short argc, t_atom *argv);

/*
 * Create and cache a zero-element Atom array.
 */
jobjectArray zeroLengthAtomArray(JNIEnv *env);

/*
 * Create and return an array of Max Atom objects using a Java array of Atom objects.
 */
t_atom *newArgv(JNIEnv *env, jobjectArray array, short *argc);

/*
 * Create and return an array of Max Atom objects using a Java array of Integer objects.
 */
t_atom *newArgvFromIntArray(JNIEnv *env, jintArray array, short *nElts);

/*
 * Create and return an array of Max Atom objects using a Java array of Float objects.
 */
t_atom *newArgvFromFloatArray(JNIEnv *env, jfloatArray array, short *nElts);

/*
 * Dump an argc/argv t_atom list to the console.
 */
void dumpArgv(short argc, t_atom *argv);

/*
 * Given a Java String, return a pointer to the corresponding Max Symbol.
 */
t_symbol *newSym(JNIEnv *env, jstring str);

#endif
