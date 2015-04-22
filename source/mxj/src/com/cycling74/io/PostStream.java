
/*
 * Copyright (c) 2001-2003, Cycling '74.  All rights reserved.
 */

package com.cycling74.io;

import com.cycling74.max.MaxObject;

import java.io.*;

/**
 * A factory
 * that returns <tt>PrintStream</tt> objects that call <tt>post</tt> to
 * write to the Max console.
 *
 * @see java.io.PrintStream
 * @see com.cycling74.io.ErrorStream
 *
 * @author Herb Jellinek
 */

public final class PostStream extends ConsoleStream {
	private static PostStream _instance = null;

    protected PostStream(ByteArrayOutputStream baos) {
	super(baos);
    }

    /**
     * Create and return a new PostStream.
     */
    public static PostStream getPostStream() {
		if(_instance == null)
			_instance = new PostStream(new ByteArrayOutputStream(16 * MAX_LINE_LENGTH));
	return _instance;
    }

    /**
     * Send the output to the console.
     */
    protected void send(String s) {
	MaxObject.post(s);
    }
}
