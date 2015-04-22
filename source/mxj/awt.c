/*
 * Windowing and UI interface to the Java AWT.
 */

#include "jni.h"
#include "jni_md.h"
#include "awt.h"
#include "maxjava.h"
#include "winproxy.h"

#ifdef USE_JMGR

#include "JManager.h"
#include "MacWindows.h"


// AWT callback protos
OSStatus requestFrame(JMAWTContextRef context,
					  JMFrameRef newFrame,
					  JMFrameKind kind,
					  const Rect *initialBounds,
					  Boolean resizeable,
					  JMFrameCallbacks *callbacks);
OSStatus releaseFrame(JMAWTContextRef context,
					  JMFrameRef oldFrame);
SInt16 uniqueMenuID(JMAWTContextRef context,
					Boolean isSubmenu);
void exceptionOccurred(JMAWTContextRef context,
						const JMTextRef exceptionName,
						const JMTextRef exceptionMsg,
						const JMTextRef stackTrace);

// Frame callback protos
static void setupFrameCallbacks(JMFrameCallbacks* callbacks);

static void setFrameSize(JMFrameRef frame,
					     const Rect *newBounds);
static void invalRect(JMFrameRef frame,
				      const Rect *dims);
static void frameShowHide(JMFrameRef frame,
					      Boolean showFrameRequested);
static void setFrameTitle(JMFrameRef frame,
					      const JMTextRef title);
static void checkUpdate(JMFrameRef frame);
static void reorderFrame(JMFrameRef frame,
					     enum ReorderRequest request);
static void setResizeable(JMFrameRef frame,
					      Boolean resizeable);
static void	getInsets(JMFrameRef frame, Rect *insets);
static void nextFocus(JMFrameRef frame, Boolean forward);
static void requestFocus(JMFrameRef frame);

// Utility functions
static char *getText(JMTextRef textRef, UInt32 *length);

static t_winproxy *getMaxProxy(JMFrameRef ref);
static OSStatus setMaxProxy(JMFrameRef ref, t_winproxy *proxy);

static t_wind *getMaxWin(JMFrameRef ref);

static t_maxjava *getAWTContextMaxObj(JMAWTContextRef ref);
static OSStatus setAWTContextMaxObj(JMAWTContextRef ref, t_maxjava *maxObj);

static void showError(const JMTextRef textRef);

extern JMAWTContextRef g_awtContext;

// AWT callbacks
JMAWTContextCallbacks g_awtCallbacks = {
	kJMVersion,
	requestFrame,
	releaseFrame,
	uniqueMenuID,
	exceptionOccurred
};

/*
 * Initialize the AWT.
 */
void initAwt(JMSessionRef jMgrSession) {
	OSErr err;
	g_awtContext = NULL;
	
	// Register the winproxy class first.
	registerWinProxy();
	
    err = JMNewAWTContext(&g_awtContext, jMgrSession, &g_awtCallbacks, 0);
    if (err != noErr || g_awtContext == NULL) {
    	error("Could not create AWT context (%d)", err);
	}
}

static const Point zeroPt = { 0, 0 };

/*
 * Request a new Frame object.
 * Sets callbacks on return.
 * Return value is a status code.
 */
OSStatus requestFrame(JMAWTContextRef context,
					  JMFrameRef newFrame,
					  JMFrameKind kind,
					  const Rect *bounds,
					  Boolean resizeable,
					  JMFrameCallbacks *callbacks) {

	OSStatus result;
	t_winproxy *winProxy;
	t_wind *maxWindow;	
	short winFlags = WCOLOR;
	// What a hack!  We assume that this is called within Max's object creation machinery
	t_maxjava *ourMaxObj = getCurrentInstance();
	
	// If ourMaxObj is null, this isn't creating an initial frame.  The Max object
	// will be stored in the context instead of having been set by the constructor.
	
	if (ourMaxObj == NULL) {
		ourMaxObj = (t_maxjava *)getAWTContextMaxObj(context);
	} else {
		OSErr err = setAWTContextMaxObj(context, (t_maxjava *)ourMaxObj);
		if (err != noErr) {
			return err;
		}
	}
	
	setupFrameCallbacks(callbacks);
	
	switch (kind) {
		case eBorderlessModelessWindowFrame:
			winFlags |= WSHADOWPROC | WGROW;
			break;
		case eModelessWindowFrame:
			winFlags |= WGROW;
			break;
		case eModalWindowFrame:
			winFlags |= WFROZEN;
			break;
	}
	
	// Create a winproxy object
	winProxy = winproxy_new();
	// Ask Max for a new window
	maxWindow = wind_new(winProxy,
						 bounds->left, bounds->top, bounds->right, bounds->bottom,
						 winFlags);
	// store pointers to window, Max object, and Frame in proxy
	winproxy_set(winProxy, ourMaxObj, maxWindow, newFrame);
	
	// set visibility to nil
	JMSetFrameVisibility(newFrame, wind_syswind(maxWindow), zeroPt, nil);
						 
	// Associate our Java Frame with the window proxy
	result = setMaxProxy(newFrame, winProxy);
	return result;
}

/*
 * Release a frame object.
 */
OSStatus releaseFrame(JMAWTContextRef context,
					  JMFrameRef oldFrame) {
	t_wind *maxWindow = getMaxWin(oldFrame);
	if (maxWindow != NULL) {
		freeobject((t_object *)maxWindow);
		setMaxProxy(oldFrame, NULL);
	}
	return noErr;
}

// Max reserves 9000-9999 for objects' menu IDs
static SInt16 g_uniqueMenuID = 9000;
#define MENUID_MAX 9999

// Max reserves 245-255 for objects' submenu IDs
static SInt16 g_uniqueSubMenuID = 245;
#define SUBMENUID_MAX 255

/*
 * Return a unique menu ID.
 */
SInt16 uniqueMenuID(JMAWTContextRef context,
					Boolean isSubmenu) {
	if (isSubmenu) {
		if (g_uniqueSubMenuID == SUBMENUID_MAX) {
			error("Ran out of submenu IDs");
			return SUBMENUID_MAX;
		} else {
			return g_uniqueSubMenuID++;
		}
	} else {
		if (g_uniqueMenuID == MENUID_MAX) {
			error("Ran out of menu IDs");
			return MENUID_MAX;
		} else {
			return g_uniqueMenuID++;
		}
	}
}

/*
 * Display a JMTextRef as an error.
 */
static void showError(const JMTextRef textRef) {
	char *str;
	UInt32 length;
	str = getText(textRef, &length);
	error("%s", str);
	freebytes(str, (short)length);
}

/*
 * Notification that an exception occurred.
 */
void exceptionOccurred(JMAWTContextRef context,
						const JMTextRef exceptionName,
						const JMTextRef exceptionMsg,
						const JMTextRef stackTrace) {
	showError(exceptionName);
	showError(exceptionMsg);
	showError(stackTrace);
}

/*
 * Set up the frame callbacks.
 */
static void setupFrameCallbacks(JMFrameCallbacks* callbacks) {
	callbacks->fVersion = kJMVersion;          // done
	callbacks->fSetFrameSize = setFrameSize;   // done; small bug
	callbacks->fInvalRect = invalRect;         //
	callbacks->fShowHide = frameShowHide;      // done
	callbacks->fSetTitle = setFrameTitle;      // done
	callbacks->fCheckUpdate = checkUpdate;     //
	callbacks->fReorderFrame = reorderFrame;   // done; bug
	callbacks->fSetResizeable = setResizeable; // done
	callbacks->fGetInsets = getInsets;         //
    callbacks->fNextFocus = nextFocus;         //
    callbacks->fRequestFocus = requestFocus;   //
}

/*
 * Set the size of the frame.  (Possible interaction with oksize msg?)
 * Can refuse by returning the old bounds.
 * XXX Java is not keeping track of the location of the Frame when the user drags it, so
 * we must.
 */
static void setFrameSize(JMFrameRef frame,
					     const Rect *newBounds) {
					     
	t_wind *window = getMaxWin(frame);
	WindowPtr macWin = wind_syswind(window);
									 
	int newWidth = newBounds->right - newBounds->left;
	int newHeight = newBounds->bottom - newBounds->top;
	
	post("setFrameSize %d,%d,%d,%d", newBounds->top, newBounds->left,
									 newBounds->bottom, newBounds->right);
									 
	if (macWin->portRect.right - macWin->portRect.left != newWidth ||
		macWin->portRect.bottom - macWin->portRect.top != newHeight) {

		SizeWindow(macWin, newWidth, newHeight, true);
		InvalRect(&macWin->portRect);
	}

	MoveWindow(macWin, newBounds->left, newBounds->top, false);
	JMSetFrameSize(frame, newBounds);
}

/*
 * Mark a region of the Frame as invalid.
 */
static void invalRect(JMFrameRef frame,
				      const Rect *dims) {
	post("invalRect %d,%d,%d,%d", dims->top, dims->left,
								  dims->bottom, dims->right);
}

/*
 * Show or hide the frame.
 */
static void frameShowHide(JMFrameRef frame,
					      Boolean showFrameRequested) {
					      
	t_wind *maxWin = getMaxWin(frame);
	post("frameShowHide %d", showFrameRequested);
	if (showFrameRequested) {
		wind_vis(maxWin);
		checkUpdate(frame);
	} else {
		wind_invis(maxWin);
	}
}

/*
 * Set the frame's title.
 */
static void setFrameTitle(JMFrameRef frame,
					      const JMTextRef title) {
	// Get the text of the title and set the window's title to it.
	UInt32 len;
	char *titleChars = getText(title, &len);
	t_wind *maxWindow = getMaxWin(frame);
	wind_settitle(maxWindow, titleChars, 0);
	freebytes(titleChars, (short)len);
}

/*
 * If an update is necessary, call JMFrameUpdate and BeginUpdate, EndUpdate -
 * or their Max equivalents (which are what?).
 */ 
static void checkUpdate(JMFrameRef frame) {
	t_wind *maxWin = getMaxWin(frame);
	WindowPtr macWin = wind_syswind(maxWin);
	if (! EmptyRgn(((WindowPeek)macWin)->updateRgn)) {
		BeginUpdate(macWin);
		SetPort(macWin);
		JMFrameUpdate(frame, macWin->visRgn);
		EndUpdate(macWin);
	}
}

/*
 * If non-modal, bring Frame to front or move to back, etc.
 * XXX need to use SelectWindow/ShowWindow?
 */
static void reorderFrame(JMFrameRef frame,
					     enum ReorderRequest request) {
	t_wind *window = getMaxWin(frame);
	WindowPtr macWin = wind_syswind(window);
	post("reorderFrame %d", request);
	switch (request) {
		case eBringToFront:        // bring the window to front
			syswindow_show(macWin);
			break;
		
        case eSendToBack:          // send the window to back
        	syswindow_hide(macWin);
	        break;
        
        case eSendBehindFront:     // send the window behind the front window
        	// not sure what to do about this
	        break;
    };
}

/*
 * Turn on or off the grow control based on the value of resizeable.
 */
static void setResizeable(JMFrameRef frame,
					      Boolean resizeable) {
	post("setResizeable %d", resizeable);
	getMaxProxy(frame)->isResizeable = resizeable;
	// XXX can we make the grow box disappear/reappear?
}

/*
 * Apple hasn't documented this.  Neat!
 */
static void	getInsets(JMFrameRef frame, Rect *insets) {
	post("getInsets");
}

/*
 * JManager.h says:
 * If the user is tabbing within a JMFrame, and the focus reaches the last focusable
 * component (or the first, if focus is traversing backwards) this function will be called.
 * The application should defocus the component that requests this, and focus the next application
 * visible focusable element.  (If none, send focus back to the frame.)
 */
static void nextFocus(JMFrameRef frame, Boolean forward) {
	post("nextFocus");
}

/*
 * JManager.h says:
 * If the AWT needs to set focus to a frame (in the case of multiple JMFrames within
 * a single Mac OS Frame) it will call back to the embedding application using
 * JMRequestFocus.  The application should then defocus what it thought did have the
 * focus, and set the focus to the new frame.
 */
static void requestFocus(JMFrameRef frame) {
	post("requestFocus");
}

//
// Utilities
//

/*
 * Get the bytes corresponding to the characters in this textRef.  Return them
 * in a freshly-allocated char buffer.
 */
static char *getText(JMTextRef textRef, UInt32 *length) {
	UInt32 textLen, numCopied;
	OSStatus err;
	char *buf;
	
	err = JMGetTextLength(textRef, &textLen);
	if (err != noErr) {
		return NULL;
	}
	buf = getbytes(textLen+1);
	err = JMGetTextBytes(textRef, kTextEncodingMacRoman, buf, textLen, &numCopied);
	if (err != noErr) {
		return NULL;
	}
	if (numCopied < textLen) {
		*length = numCopied;
	} else {
		*length = textLen;
	}
	buf[*length] = '\0';
	return buf;
}

/*
 * Return the window proxy corresponding to this JMFrameRef.
 */
static t_winproxy *getMaxProxy(JMFrameRef ref) {
	JMClientData result;
	if (JMGetFrameData(ref, &result) == noErr) {
		return (t_winproxy *)result;
	}
	return NULL;
}

/*
 * Store in the Frame the Max window corresponding to it.
 */
static OSStatus setMaxProxy(JMFrameRef ref, t_winproxy *winProxy) {
	return JMSetFrameData(ref, (JMClientData)winProxy);
}

/*
 * Return the Max window corresponding to this JMFrameRef.
 */
static t_wind *getMaxWin(JMFrameRef ref) {
	JMClientData result;
	if (JMGetFrameData(ref, &result) == noErr) {
		return ((t_winproxy *)result)->window;
	}
	return NULL;
}

/*
 * Return the Max object corresponding to this JMAWTContextRef.
 */
static t_maxjava *getAWTContextMaxObj(JMAWTContextRef ref) {
	JMClientData result;
	if (JMGetAWTContextData(ref, &result) == noErr) {
		return (t_maxjava *)result;
	}
	return NULL;
}

/*
 * Store in the AWT Context the Max object corresponding to it.
 */
static OSStatus setAWTContextMaxObj(JMAWTContextRef ref, t_maxjava *maxObj) {
	return JMSetAWTContextData(ref, (JMClientData)maxObj);
}


/*
 * The MaxJava instance currently being constructed.
 */
static t_maxjava *g_currentMaxObj;

/*
 * Set the current MaxJava instance.  This is called by maxjava.c.
 */
void proclaimInstance(t_maxjava *instance) {
	g_currentMaxObj = instance;
}

/*
 * Get the current MaxJava instance.
 */
t_maxjava *getCurrentInstance(void) {
	return g_currentMaxObj;
}

/*
 * Forget the current instance.
 */
void resetInstance() {
	g_currentMaxObj = NULL;
}


#endif
