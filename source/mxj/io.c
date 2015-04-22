/*
 * io.c - Simple interface from the MRJ (Java) I/O system to Max.
 *
 * Author: Herb Jellinek
 */

#include "io.h"

#ifdef USE_JMGR

/*
 * This will serve as glue between System.err/System.out and the Max console.
 */
void sysOutHandler(JMSessionRef session, const void* msg, SInt32 length) {
	// We expect this to be in kTextEncodingMacRoman
	char *message = (char *)msg;
	message[length] = 0;
	post("%s", message);
}

/*
 * Called when Java calls System.exit(value).  If we return true, the current MacOS thread
 * is killed.  If false, the OS sends Max a QUIT event.  Let's return false, eh wot?
 */
Boolean sysExitHandler(JMSessionRef session, SInt32 value) {
	return false;
}

#endif
