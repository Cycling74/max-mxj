package com.cycling74.max;

import java.io.*;

/**
 * Collection of functions relating primarily to interaction with the Max
 * environment and other information about the environment mxj is executing
 * within.
 * 
 * @author Topher LaFata, Ben Nevile
 */

public class MaxSystem {

	private static final String[] _sys_classpath = getSystemClassPath();

	/**
	 * Defer the execution of fn to the low priority cue. If the calling thread
	 * is the low priority thread fn is executed immediately. Defer places the
	 * task on the front of the low priority queue. In some instances this
	 * behavior can result in the reversal of scheduled tasks. In these cases
	 * use deferLow if that is not the desired behavior.
	 * 
	 * @param fn
	 *            is the instance of Executable to be executed by the low
	 *            priority thread.
	 */
	public static native void defer(Executable fn);

	/**
	 * Defer the execution of fn to the back of the low priority queue
	 * regardless whether or not the calling thread is the low or high priority
	 * thread(scheduler). Thus ordering of tasks is always preserved.
	 * 
	 * @param fn
	 *            is the instance of Executable to be executed by the low
	 *            priority thread.
	 */
	public static native void deferLow(Executable fn);

	/**
	 * Defer the execution of fn to the back of the low priority queue if the
	 * calling thread is the high priority thread(scheduler). Otherwise, execute
	 * fn immediately.
	 * 
	 * @param fn
	 *            is the instance of Executable to be executed by the low
	 *            priority thread.
	 */
	public static native void deferMedium(Executable fn);

	/**
	 * Defer the execution of fn to the front of the low priority queue
	 * regardless whether or not the calling thread is the low or high priority
	 * thread(scheduler).
	 * 
	 * @param fn
	 *            is the instance of Executable to be executed by the low
	 *            priority thread.
	 */
	public static native void deferFront(Executable fn);

	/**
	 * Schedule the execution of fn at an absolute time in the future by the
	 * high priority thread.
	 * 
	 * @param fn
	 *            is the instance of Executable to be executed by the scheduler
	 *            thread.
	 * @param abs_time
	 *            the absolute time when at which fn should be executed.
	 */
	public static native void schedule(Executable fn, double abs_time);

	/**
	 * Schedule the execution of fn delay millis from now by the high priority
	 * thread.
	 * 
	 * @param fn
	 *            is the instance of Executable to be executed by the scheduler
	 *            thread.
	 * @param delay
	 *            the offset in millis from now at which fn should be executed.
	 */
	public static native void scheduleDelay(Executable fn, double delay);

	/**
	 * Creates a clock to execute at some time in the future. When the clock
	 * executes, it calls defer (hence creating a qelem to execute at low
	 * priority).
	 * 
	 * @param fn
	 *            is the instance of Executable to be executed by the scheduler
	 *            thread.
	 * @param delay
	 *            the offset in millis from now at which fn should be executed.
	 */
	public static native void scheduleDefer(Executable fn, double delay);

	/**
	 * @return true if the calling thread is either the max main application
	 *         (low priority) thread or the max scheduler(high priority) thread.
	 */
	public static native boolean inMaxThread();

	/**
	 * @return true if the calling thread is the max scheduler (high priority)
	 *         thread.
	 */
	public static native boolean inTimerThread();

	/**
	 * @return true if the calling thread is the max main (low priority) thread.
	 */
	public static native boolean inMainThread();

	/**
	 * Show the cursor via native OS call.
	 */
	public static native void showCursor();

	/**
	 * Hide the cursor via native OS call.
	 */
	public static native void hideCursor();

	/**
	 * This is a workaround for a bug in swing on OSX jaguar. Swing dialogs
	 * cause the app to become stuck in a modal state. If you call this function
	 * before showing the showing dialog the app is able to recover properly.
	 */
	public static native void nextWindowIsModal();

	/**
	 * This method provides and easy way for mxj objects to communicate with
	 * other max objects or libraries written in C by passing messages. Objects
	 * must be bound to the s_thing of a global symbol generated via a call to
	 * gensym. Messages cannot be declared as A_CANT since typedmess is used for
	 * the dispatching. Returns true on success, false otherwise. IN C:
	 * 
	 * <pre>
	 * 
	 * 	 	void main(void)
	 * 	 	{
	 * 	 		...blah 
	 * 	 		addmess((method)my_obj_setstate, "setstate", A_GIMME, 0)
	 * 	 		..blah
	 * 	 	}
	 * 	 
	 * 	 	t_my_obj *x;
	 * 		blah blah...
	 * 		gensym("__my_special_lib__")->s_thing = (t_object *)x;
	 * </pre>
	 * 
	 * IN JAVA:
	 * 
	 * <pre>
	 * 		private static final String LIBNAME = "__my_special_lib__";
	 * 		private static final int STATE_INIT = 0;
	 * 		private static final int STATE_ACTIVE = 1;
	 * 		
	 * 		blah blah..
	 * 		
	 * 		public void setBackendState()
	 * 		{
	 * 			Atom[] args = new Atom[1];
	 * 			args[0]= Atom.newAtom(STATE_INIT);
	 * 			boolean res = MaxSystem.sendMessageToBoundObject(LIBNAME,"setstate",args);
	 * 			...
	 * 		}
	 * </pre>
	 * 
	 * Args can be null if message takes no arguments.
	 * 
	 * @return true if message was sent successfully.
	 * @param sym_name
	 *            the symbol to which the object is bound
	 * @param msg
	 *            the message to pass
	 * @param args
	 *            arguments to pass with the message
	 */
	public static native boolean sendMessageToBoundObject(String sym_name,
			String msg, Atom[] args);

	/**
	 * Shows an open file dialog using the Max API.
	 * 
	 * @return full path of file to open, returns null if cancelled.
	 */
	public static String openDialog() {
		return openDialog(null);
	}

	/**
	 * Shows an open file dialog using the Max API.
	 * 
	 * @param prompt
	 *            message to display in dialog.
	 * @return full path of file to open, returns null if cancelled.
	 */
	public static native String openDialog(String prompt);

	/**
	 * Shows a save as.. file dialog using the Max API.
	 * 
	 * @param prompt
	 *            message to display in dialog.
	 * @param default_filename
	 *            default file chosen when dialog is shown.
	 * @return full path of location to save to, returns null if cancelled.
	 */
	public static native String saveAsDialog(String prompt,
			String default_filename);

	/**
	 * Get the absolute path of first file named filename found in the MAX
	 * search path
	 * 
	 * @param filename
	 *            file to look for in Max search path
	 * @return absolute path of first file found named filname in Max search
	 *         path.
	 */
	public static native String locateFile(String filename);

	/**
	 * Get the absolute path of the current default directory of the Max
	 * application.
	 * 
	 * @return The current default directory of the Max application.
	 */
	public static native String getDefaultPath();

	/**
	 * Get the absolute path of the preferences directory of the Max
	 * application.
	 * 
	 * @return The preferences directory of the Max application.
	 */
	public static native String getPreferencesPath();

	/**
	 * Get the current Max search path list as an array of Strings where each
	 * String is an absolute path to a directory in the current Max search path.
	 * 
	 * @return The search path list of the Max application.
	 */
	public static native String[] getSearchPath();

	/**
	 * Get the contextual Max search path list as an array of Strings where each
	 * String is an absolute path to a directory in the contextual (project- or
	 * collective-specific) search path.
	 * 
	 * @return The contextual search path list of the current special context,
	 *         if available.
	 */
	public static native String[] getSearchPathForContext();

	/**
	 * Used with the nameConform method. As of version 4.3, Max’s path style is
	 * the slash style. Max style paths are of the form vol:/path/to/file
	 */
	public static final int PATH_STYLE_MAX = 0;
	/**
	 * Used with the nameConform method. This is the path style native to the
	 * operating system.
	 */
	public static final int PATH_STYLE_NATIVE = 1;
	/**
	 * Used with the nameConform method. The Mac OS 9 path style used by Max 4.1
	 * and earlier.
	 */
	public static final int PATH_STYLE_COLON = 2;
	/**
	 * Used with the nameConform method.The cross-platform path style used by
	 * Max 4.3 and later.
	 */
	public static final int PATH_STYLE_SLASH = 3;
	/**
	 * Used with the nameConform method.The Windows backslash path style (not
	 * recommended for outletting, since the backslash is a special character in
	 * Max).
	 */
	public static final int PATH_STYLE_NATIVE_WIN = 4;
	/**
	 * Used with the nameConform method.Do not use a path type.
	 */
	public static final int PATH_TYPE_IGNORE = 0;
	/**
	 * Used with the nameConform method.Do not use a path type.
	 */
	public static final int PATH_TYPE_ABSOLUTE = 1;
	/**
	 * Used with the nameConform method.A path relative to the Max application
	 * folder.
	 */
	public static final int PATH_TYPE_RELATIVE = 2;
	/**
	 * Used with the nameConform method.A path relative to the boot volume
	 */
	public static final int PATH_TYPE_BOOT = 3;
	/**
	 * Used with the nameConform method.A path relative to the Cycling 74 app
	 * support folder.
	 */
	public static final int PATH_TYPE_C74 = 4;

	/**
	 * change format of a path from one style/type to another.
	 * 
	 * @param filename
	 *            old path
	 * @param style
	 *            path style
	 * @param type
	 *            path type
	 * @return altered path String
	 */
	public static native String nameConform(String filename, int style, int type);

	/**
	 * Convert a max style path (vol:/path/to/some/file) to a path native to the
	 * current OS.
	 * 
	 * @param maxpath
	 *            the path to convert
	 * @return String that repesents path native to the current OS
	 */
	public static native String maxPathToNativePath(String maxpath);

	public static native double sysTimerGetTime();

	/**
	 * Get the version number of the Max system. The version is encoded as
	 * follows:
	 * 
	 * <ul>
	 * <li>Bit 14 (counting from the left) is set if Max is running as a
	 * standalone program.
	 * <li>The lower 12 bits represent the version number in BCD (binary coded
	 * decimal).
	 * </ul>
	 * For example, if the lower bits are x406, this is Max version 4.0.6.
	 * 
	 * @return encoded version of Max application.
	 */
	public static short getMaxVersion() {
		return doGetMaxVersion();
	}

	/**
	 * @return true if we are running inside of the max runtime as opposed to
	 *         the Max/MSP application.
	 * 
	 */
	public static boolean isRuntime() {
		return (doGetIsRuntime());
	}

	/**
	 * @return true if we are running inside of a standalone app as opposed to
	 *         the max runtime or Max/MSP.
	 * 
	 */
	public static boolean isStandAlone() {
		return ((0x2000 & doGetMaxVersion()) != 0);
	}

	/**
	 * A utility method to get the current version of max as an array of ints.
	 * 
	 * @return three element integer array: [major, minor, update]
	 */
	public static int[] getMaxVersionInts() {
		int[] i = new int[3];
		short v = doGetMaxVersion();
		i[0] = (v & 0x0f00) >>> 8;
		i[1] = (v & 0x00f0) >>> 4;
		i[2] = (v & 0x000f);
		return i;
	}

	private static final native short doGetMaxVersion();

	public static native boolean doGetIsRuntime();

	/**
	 * @return true if we are running in Windows.
	 */
	public static boolean isOsWindows() {
		String osName = System.getProperty("os.name", "");
		return (osName.startsWith("Windows"));
	}

	/**
	 * @return true if we are running in OS X.
	 */
	public static boolean isOsMacOsX() {
		String osName = System.getProperty("os.name", "");
		return (osName.startsWith("Mac OS X"));
	}

	/**
	 * @return the system classpath i.e the immutable classpath as an array of
	 *         Strings
	 */
	public static String[] getSystemClassPath() {
		if (_sys_classpath != null)
			return _sys_classpath;
		else
			return (System.getProperty("java.class.path")).split(System
					.getProperty("path.separator"));
	}

	/**
	 * @return sys and dynamic classpath as an array of Strings.
	 */
	public static String[] getClassPath() {
		int syscplen = getSystemClassPath().length;
		String[] tmp = MXJClassLoader.getInstance().getDynamicClassPath();
		String[] ret = new String[tmp.length + syscplen];

		int i = 0;
		for (i = 0; i < syscplen; i++)
			ret[i] = _sys_classpath[i];
		for (i = syscplen; i < ret.length; i++)
			ret[i] = tmp[i - syscplen];

		return ret;
	}

	/**
	 * The interaction between Max/MSP and Swing/AWT Frames is a bit complex on
	 * OS X. This method provides a workaround for Max/MSP allowing your
	 * Swing/AWT frame based interface to have a menu which responds to hotkeys.
	 * This is how the mxj editor accomplishes its behavior with regards to
	 * command-w closing an editor window and command-o opening a file open
	 * dialog etc. Max needs to know which menu items to disable on itself when
	 * your AWT/Swing interface is in focus. This method allows you to specify
	 * which command key accelerators max should disable in its own application
	 * menus when your custom AWT/Swing frame has focus.
	 * 
	 * <PRE>
	 * // from MXJEditor.java
	 * // call before frame is visible.
	 * private static final char[] __accs = new char[] { 'X', 'C', 'V', 'S', 'Z', 'O',
	 * 		'N', 'W', 'F', 'G', 'H', 'L', 'K', 'D' };
	 * com.cycling74.max.MaxSystem.registerCommandAccelerators(__accs);
	 * </PRE>
	 * 
	 * @param c
	 *            array of command accelerators.
	 */
	public static void registerCommandAccelerators(char[] c) {
		for (int i = 0; i < c.length; i++) {
			MaxSystem.registerCommandAccelerator(c[i]);
		}
	}

	/**
	 * The interaction between Max/MSP and Swing/AWT Frames is a bit complex on
	 * OS X. This method provides a workaround for Max/MSP allowing your
	 * Swing/AWT frame based interface to have a menu which responds to hotkeys.
	 * This is how the mxj editor accomplishes its behavior with regards to
	 * command-w closing an editor window and command-o opening a file open
	 * dialog etc. Max needs to know which menu items to disable on itself when
	 * your AWT/Swing interface is in focus. This method allows you to specify a
	 * command key accelerator that max should disable in its own application
	 * menus when your custom AWT/Swing frame has focus. For instance if you
	 * need to respond to cmnd - o(mac) or cntrl-o(pc) you would register char
	 * 'o'. That way your window would get the cmnd-o instead of the Max
	 * application which would cause Max to open a file open dialog.
	 * 
	 * @param c
	 *            command accelerator to disable when your AWT/Swing Frame has
	 *            focus.
	 */
	public static void registerCommandAccelerator(char c) {
		_register_command_accelerator(c);
	}

	private static native void _register_command_accelerator(char c);

	/**
	 * Unregister command accelerators.
	 * 
	 * @param c
	 *            array of command accelerators
	 */
	public static void unregisterCommandAccelerators(char[] c) {
		for (int i = 0; i < c.length; i++) {
			MaxSystem.unregisterCommandAccelerator(c[i]);
		}
	}

	private static native void _unregister_command_accelerator(char c);

	/**
	 * Unregister a command accelerator.
	 * 
	 * @param c
	 *            the command accelerator
	 */
	public static void unregisterCommandAccelerator(char c) {
		_unregister_command_accelerator(c);
	}

	/**
	 * Post a message to the Max console.
	 * 
	 * @param message
	 *            the <code>String</code> to post in the Max console.
	 */
	public static void post(String message) {
		try {
			if (message != null) {
				if (message.length() > 1023) {
					message = message.substring(0, 1023);
				}
				byte msgBytes[] = message.getBytes("UTF-8");
				doPost(msgBytes);
			}
		} catch (IOException ioe) {
			MaxSystem.error("io error." + ioe);
		}
	}

	/**
	 * Force loads a max external for things like Jitter Java support.
	 * 
	 * @param objname
	 *            the <code>String</code> of the max object name.
	 */
	public static void loadObject(String objname) {
		try {
			if (objname != null) {
				if (objname.length() > 1023) {
					objname = objname.substring(0, 1023);
				}
				byte msgBytes[] = objname.getBytes("UTF-8");
				doLoadObject(msgBytes);
			}
		} catch (IOException ioe) {
			MaxSystem.error("io error." + ioe);
		}
	}

	/**
	 * Post an error message to the Max console.
	 * 
	 * @param message
	 *            the <code>String</code> to post as an error to the Max
	 *            console.
	 */
	public static void error(String message) {
		try {
			if (message != null) {
				if (message.length() > 1023) {
					message = message.substring(0, 1023);
				}
				byte msgBytes[] = message.getBytes("UTF-8");
				doError(msgBytes);
			}
		} catch (IOException ioe) {
			MaxSystem.error("io error." + ioe);
		}
	}

	/**
	 * Put up an error window. Extremely annoying. Should probably never be
	 * used. I'm not even sure why we support it.
	 * 
	 * @param message
	 *            the text to display in the error window.
	 */
	public static void ouch(String message) {
		try {
			byte msgBytes[] = message.getBytes("UTF-8");
			doOuch(msgBytes);
		} catch (IOException ioe) {
			MaxSystem.error("io error." + ioe);
		}
	}

	/**
	 * Post a message to the Max console. Internal method.
	 */
	private static final native void doPost(byte message[]);

	/**
	 * Force loads a max external for things like Jitter Java support. Internal
	 * method.
	 */
	private static final native void doLoadObject(byte message[]);

	/**
	 * Post an error message to the Max console. Max objects can subscribe to
	 * error messages. Internal method.
	 */
	private static final native void doError(byte message[]);

	/**
	 * Put up a Max error window. Internal method.
	 */
	private static final native void doOuch(byte message[]);

}
