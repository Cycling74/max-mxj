
/*
 * Copyright (c) 2001-2003, Cycling '74.  All rights reserved.
 */

package com.cycling74.io;

import com.cycling74.max.MaxObject;

import java.io.*;

/**
 * A factory
 * that returns <tt>PrintStream</tt> objects that call <tt>error</tt> to
 * write to the Max console.
 *
 * @see java.io.PrintStream
 * @see com.cycling74.io.PostStream
 *
 * @author Herb Jellinek
 */ 

public final class ErrorStream extends ConsoleStream {
	private static ErrorStream _instance = null;

    protected ErrorStream(ByteArrayOutputStream baos) {
	super(baos);
    }

    /**
     * Create and return a new ErrorStream.
     */
    public static ErrorStream getErrorStream() {
	    if(_instance == null)
			_instance = new ErrorStream(new ByteArrayOutputStream(16 * MAX_LINE_LENGTH));
		return _instance;
    }

    /**
     * Send the output to the console.
     */
    protected void send(String s) {
	MaxObject.error(s);
    }
}
