/*
 * Headers for awt.c.
 */


#ifndef _Included_awt_h
#define _Included_awt_h

#include "maxjava.h"
#include "JManager.h"

#ifdef USE_JMGR

void initAwt(JMSessionRef ref);

void proclaimInstance(t_maxjava *instance);
t_maxjava *getCurrentInstance(void);
void resetInstance(void);

OSStatus requestFrame(JMAWTContextRef context,
					  JMFrameRef newFrame,
					  JMFrameKind kind,
					  const Rect *bounds,
					  Boolean resizeable,
					  JMFrameCallbacks *callbacks);

OSStatus releaseFrame(JMAWTContextRef context,
					  JMFrameRef oldFrame);

#endif

#endif
