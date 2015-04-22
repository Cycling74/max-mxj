/*
 * winproxy.c - a proxy that mediates between Max Windows and Max objects, allowing
 * Max objects to maintain multiple windows and know which one received a message.
 *
 * Author: Herb Jellinek
 */

#include "winproxy.h"
#include "classops.h"


// The class object
static Maxclass g_winproxy_class;
// The message list
#define NUM_MESSAGES 11
static Messlist g_winproxy_messList[NUM_MESSAGES+1];
// The instance free list
static Object *g_winproxy_freelist;



// Window message methods
void winproxy_click(t_winproxy *x, Point pt, short dblClick, short modifiers);
void winproxy_update(t_winproxy *x);
void winproxy_key(t_winproxy *x, short key, short modifiers, short keyCode);
void winproxy_idle(t_winproxy *x, Point mouseLoc, short within);
void winproxy_activate(t_winproxy *x, short active);
void winproxy_close(t_winproxy *x);
void winproxy_scroll(t_winproxy *x);
void winproxy_vis(t_winproxy *x);
void winproxy_invis(t_winproxy *x);
void winproxy_oksize(t_winproxy *x, short *hsize, short *vsize);
void winproxy_wsize(t_winproxy *x, short hsize, short vsize);


void registerWinProxy(void) {
	// load this object into Max's memory so it can be used in a patch
	mxj_maxclass_new("winproxy", sizeof(t_winproxy), (method)winproxy_free, &g_winproxy_class,
			  &g_winproxy_freelist);
	bindWinMessages();
}

/*
 * Create a winproxy object.
 */
t_winproxy *winproxy_new() {
	t_winproxy *proxy;
	
	proxy = newobject(g_winproxy_messList);
	proxy->isResizeable = true;
	
	return proxy;
}

/*
 * Free this instance.
 */
void winproxy_free(t_winproxy *x) {
	// not much to do but some cleaning up
	if (x->window != NULL) {
		freeobject((Object *)(x->window));
	}
	winproxy_set(x, NULL, NULL, NULL);
}

/*
 * Associate an owning Max object with a window.
 */
void winproxy_set(t_winproxy *self, void *owner, t_wind *window, JMFrameRef frame) {
	self->ownerObj = owner;
	self->window = window;
	self->frame = frame;
}

/*
 * Bind messages to methods.
 */
void bindWinMessages() {
	char *messNames[] = {
		"click",
		"update",
		"key",
		"idle",
		"activate",
		"close",
		"scroll",
		"vis",
		"invis",
		"oksize",
		"wsize",
	};
	
	method meths[] = {
		(method)winproxy_click,
		(method)winproxy_update,
		(method)winproxy_key,
		(method)winproxy_idle,
		(method)winproxy_activate,
		(method)winproxy_close,
		(method)winproxy_scroll,
		(method)winproxy_vis,
		(method)winproxy_invis,
		(method)winproxy_oksize,
		(method)winproxy_wsize,
	};
	
	mxj_maxclass_addmethods(&g_winproxy_class, g_winproxy_messList, NUM_MESSAGES,
					 messNames, meths);
}

//
// Window message methods
//
void winproxy_click(t_winproxy *proxy, Point pt, short dblClick, short modifiers) {
	// XXX do we need to test to see if we're the frontmost window?
	// XXX what about dblClick?
	JMFrameRef frame = proxy->frame;
	t_wind *window = proxy->window;
	WindowPtr macWin = wind_syswind(window);

	if (frame) {
		SetPort(macWin);
		JMFrameClick(frame, pt, modifiers);
		post("called JMFrameClick");
	}
	post("click %ld", proxy);
}

void winproxy_update(t_winproxy *proxy) {
	JMFrameRef frame = proxy->frame;
	t_wind *wind = proxy->window;
	WindowPtr macWin = wind_syswind(wind);
	post("update %ld", proxy);
	if (frame) {
		JMFrameUpdate(frame, macWin->visRgn);
		post("called JMFrameUpdate");
	} else {
		EraseRgn(macWin->visRgn);
		post("no frame?");
	}
}

void winproxy_key(t_winproxy *proxy, short key, short modifiers, short keyCode) {
	post("key %ld", proxy);
}

void winproxy_idle(t_winproxy *proxy, Point mouseLoc, short within) {
//	post("idle %ld", proxy);
}

void winproxy_activate(t_winproxy *proxy, short active) {
	JMFrameRef frame = proxy->frame;
	post("activate %ld", proxy);
	if (frame) {
		JMFrameActivate(frame, active);
		post("called JMFrameActivate");
	}
}

/*
 * The window is closing, so let's pack our bags.
 */
void winproxy_close(t_winproxy *proxy) {
	freeobject((t_object *)proxy);
}

void winproxy_scroll(t_winproxy *proxy) {
	post("scroll %ld", proxy);
}

void winproxy_vis(t_winproxy *proxy) {
	post("vis %ld", proxy);
}

void winproxy_invis(t_winproxy *proxy) {
	post("invis %ld", proxy);
}

/*
 * Called when the user has resized the window.  Determine if the proposed new size is OK.
 */
void winproxy_oksize(t_winproxy *proxy, short *hsize, short *vsize) {
	post("oksize %d,%d", *hsize, *vsize);
}

/*
 * Called when the user has resized the window.  Notify the AWT
 * of the Frame's new dimensions.
 */
void winproxy_wsize(t_winproxy *proxy, short hsize, short vsize) {
	t_wind *window = proxy->window;
	WindowPtr macWin = wind_syswind(window);
	JMFrameRef frame = proxy->frame;
	short oldLeft = macWin->portRect.left;
	short oldTop = macWin->portRect.top;
	Rect newBounds;
	
	SetRect(&newBounds, oldLeft, oldTop, oldLeft + hsize, oldTop + vsize);
	
	post("wsize %d,%d,%d,%d", oldLeft, oldTop, oldLeft + hsize, oldTop + vsize);
	JMSetFrameSize(frame, &newBounds);
}


