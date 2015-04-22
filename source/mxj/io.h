/*
 * Headers for io.c.
 */


#ifndef _Included_io_h
#define _Included_io_h

#include "whichjava.h"


#ifdef USE_JMGR
void sysOutHandler(JMSessionRef session, const void* msg, SInt32 length);
Boolean sysExitHandler(JMSessionRef session, SInt32 value);
#endif


#endif
