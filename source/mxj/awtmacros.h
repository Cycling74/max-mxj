/*
 * Macros for defining and binding window message functions.
 */


#ifndef _Included_awtmacros_h
#define _Included_awtmacros_h

// Bind a method of an object to the message of the same name (suffix).
// For example, BIND(maxjava, click); turns into
// addmess(maxjava_click, "click", A_CANT, 0);
#define BIND(objName, msgName) addmess((method)objName##_##msgName, #msgName"", A_CANT, 0)

#endif
