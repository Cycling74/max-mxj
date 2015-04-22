/*
 *  Interface between Max and JavaEmbedding framework.
 */

#include <Carbon/Carbon.h>
#include <JavaEmbedding/JavaEmbedding.h>

#include "mxjawtinit.h"
#include "classpath.h"
#include "threadenv.h"
#include "dbg.h"

#pragma export on

int jvm_new();
int initAWT(JavaVM *jvm, JNIEnv *env, CFBundleRef thisBundleRef);
int killApplet(void);
static pascal OSStatus winEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* userData);
void runApplet(JNIEnv *env, CFURLRef codeBase);
void ext_main(void *r);

#pragma export reset

#define DEBUG TRUE

static JavaVM *g_jvm = NULL; // the single Java virtual machine we will create

static jobject g_applet = NULL;
static ControlRef g_control = NULL;
static WindowRef g_window = NULL;

pascal OSStatus winEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* userData) {
#pragma unused (userData)
	UInt32 eventKind;
	OSStatus err;
	
	JNIEnv *env = NULL;
	THREADENV(env);
		
	eventKind = GetEventKind(inEvent);
		
	if (eventKind == kEventWindowClose) {		
		killApplet();
	} else if (eventKind == kEventWindowBoundsChanged) {
		MoveAndClipJavaControl(env, g_control, APPLET_X, APPLET_Y, APPLET_X, APPLET_Y,
							   APPLET_WIDTH, APPLET_HEIGHT);
		DrawJavaControl(env, g_control);
	}
	err = CallNextEventHandler(inHandlerCallRef, inEvent);
	return err;
}

void runApplet(JNIEnv *env, CFURLRef codeBase) {
	OSStatus err;
	int numValues = 0;
	CFStringRef keys[256]		= { 0 };
	CFStringRef values[256]		= { 0 };
	AppletDescriptor appletDescriptor;
	WindowAttributes windowAttrs;
	Rect contentRect;
	
	// We're creating the tag
	// <applet archive="APPLET_JAR" code="APPLET_NAME" width="APPLET_WIDTH_STR"
	//         height="APPLET_HEIGHT_STR">
	// </applet>
	keys[numValues] =     CFSTR("archive");
	values[numValues++] = CFSTR(APPLET_JAR_NAME".jar");
	keys[numValues] =     CFSTR("code");
	values[numValues++] = CFSTR(APPLET_NAME);
	keys[numValues] =     CFSTR("width");
	values[numValues++] = CFSTR(APPLET_WIDTH_STR);
	keys[numValues] =     CFSTR("height");
	values[numValues++] = CFSTR(APPLET_HEIGHT_STR);

	appletDescriptor.codeBase = codeBase;
	appletDescriptor.docBase = codeBase;
	appletDescriptor.htmlAttrs =
		CFDictionaryCreate(NULL, (const void**) keys,
						   (const void**) values, numValues, NULL, NULL);

	appletDescriptor.appletParams =
		CFDictionaryCreate(NULL, NULL, NULL, 0, NULL, NULL);

	if ((err = CreateJavaApplet(env, appletDescriptor, true, NULL, &g_applet)) == noErr) {
	
		windowAttrs = kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute |
					  kWindowInWindowMenuAttribute;
		SetRect(&contentRect, APPLET_X, APPLET_Y,
			    APPLET_X + APPLET_WIDTH, APPLET_Y + APPLET_HEIGHT);
			    
		if ((err = CreateNewWindow(kDocumentWindowClass, windowAttrs,
								   &contentRect, &g_window)) == noErr) {
			Rect bounds;
			EventTypeSpec spec[] = {{ kEventClassWindow, kEventWindowClose },
									{ kEventClassWindow, kEventWindowBoundsChanged }};
			UInt32 eventTypeCount = sizeof(spec) / sizeof(EventTypeSpec);
			
			InstallWindowEventHandler(g_window, NewEventHandlerUPP(winEventHandler), eventTypeCount,
									  (EventTypeSpec*)&spec, NULL, NULL);

			GetWindowBounds(g_window, kWindowContentRgn, &bounds);
			OffsetRect(&bounds, -bounds.left, -bounds.top);
// Fix for bug #622!  Until such time as we figure out a more clever title than none.
//			SetWindowTitleWithCFString (g_window, CFSTR(APPLET_NAME));

			if ((err = CreateJavaControl(env, g_window, &bounds, g_applet, true, &g_control)) == noErr) {
				SetJavaAppletState(env, g_applet, kAppletStart);
				ShowWindow(g_window);
				ShowHideJavaControl(env, g_control, 1);
			}
		}
	}
	
	CFRelease(appletDescriptor.htmlAttrs);
	CFRelease(appletDescriptor.appletParams);
}

/*
 * Called from mxj.
 */
int initAWT(JavaVM *jvm, JNIEnv *env, CFBundleRef thisBundleRef) {
	CFURLRef resources = CFBundleCopyResourcesDirectoryURL(thisBundleRef);
	CFURLRef codeBase =
		CFURLCreateCopyAppendingPathComponent(NULL,
											  CFURLCopyAbsoluteURL(resources),
											  CFSTR("Java"),
											  true);
/*
	CFURLRef codeBase = CFBundleCopyResourceURL(thisBundleRef,
												CFSTR(APPLET_JAR_NAME),
												CFSTR("jar"),
												CFSTR("Java"));
*/
	g_jvm = jvm;
	
	(*env)->ExceptionClear(env);
	runApplet(env, codeBase);
	(*env)->ExceptionClear(env);
	
	CFRelease(codeBase);
	return noErr;
}

/*
 * Called from mxj or event handler.  Jane, stop this crazy thing!
 */
int killApplet() {
	if (g_control != NULL) {
		JNIEnv *env = NULL;
		THREADENV(env);
		
		StopJavaControlAsyncDrawing(env, g_control);
		ShowHideJavaControl(env, g_control, 0);
		SetJavaAppletState(env, g_applet, kAppletStop);
		SetJavaAppletState(env, g_applet, kAppletDestroy);
		DisposeControl(g_control);
		DisposeWindow(g_window);
		g_control = NULL;
		g_applet = NULL;
		g_window = NULL;
		(*env)->ExceptionClear(env);
	}
	return noErr;
}

/*
 * Keep the linker happy.
 */
void ext_main(void *r) {
}
