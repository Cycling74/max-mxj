#ifndef _Included_winproxy_h
#define _Included_winproxy_h


#include "jni.h"
#include "jni_md.h"
#ifdef USE_JMGR
#include "JManager.h"
#include "MacWindows.h"
#endif

typedef struct winproxy {
	t_object p_ob;		// object header
	
	t_wind *window;     // the Max window we correspond to
	void *ownerObj;     // the Max object that owns it
	JMFrameRef frame;   // the corresponding Java Frame
	
	// Flags and fields
	char isResizeable;  // can it be resized?

} t_winproxy;


void registerWinProxy(void);

t_winproxy *winproxy_new(void);
void winproxy_free(t_winproxy *x);
void winproxy_set(t_winproxy *self, void *owner, t_wind *window, JMFrameRef frame);

void bindWinMessages(void);

#endif
