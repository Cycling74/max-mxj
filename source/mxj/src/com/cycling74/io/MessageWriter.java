
/*
 * Copyright (c) 2002-2003, Cycling '74.  All rights reserved.
 */

package com.cycling74.io;

import com.cycling74.max.*;

import java.io.*;

/**
 * A <tt>Writer</tt> that sends messages through one of an object's outlets.
 * The <tt>Writer</tt> interprets a newline character as the signal to flush
 * its buffer and produce output.
 *
 * @author Herb Jellinek
 *
 * @see java.io.Writer
 */

public class MessageWriter extends Writer {

    /**
     * The message for the IOException we generate when writing to a closed
     * instance.
     */
    private static final String WRITER_CLOSED = "Writer is closed";

    /**
     * The End of line character.
     */
    private static final char EOL = '\n';

    /**
     * The object that will be doing our outlet() calls.
     */
    private MaxObject mObj = null;

    /**
     * The index of the outlet to use.
     */
    private int mOutlet = 0;

    /*
     * The backing buffer.
     */
    private StringBuffer mBuf = new StringBuffer(256);

    /**
     * Create a Writer that sends its output to an object's outlet.
     */
    public MessageWriter(MaxObject obj, int outlet) {
	super();
	mObj = obj;
	mOutlet = outlet;
    }

    /**
     * Close the stream, flushing it first.
     * Set the backing buffer to null.
     */
    public void close() throws IOException {
	synchronized (mBuf) {
	    if (mBuf != null) {
		// exception is pointless when closing a closed Writer
		flush();
		mBuf = null;
	    }
	}
    }

    /**
     * If accessing a closed one of these, throw an IOException.
     */
    private void checkOpen() throws IOException {
	if (mBuf == null) {
	    throw new IOException(WRITER_CLOSED);
	}
    }	

    /**
     * Flush the stream.  Output all buffer contents and reset the buffer
     * pointer to 0.
     * Assume that the buffer contains a single output line of text, not
     * terminated by an EOL.
     */
    public void flush() throws IOException {

	synchronized (mBuf) {
	    checkOpen();

	    if (mBuf.length() == 0) {
		return;
	    }
	    
	    String str = mBuf.toString();
	    
	    // is there a single token?
	    int spacePos = str.indexOf(' ');
	    
	    if (spacePos == -1) {	// yes, a single token
		// if token is interpretable as an int, send an int.
		try {
		    int possibleInt = Integer.parseInt(str);
		    
		    // send possibleInt
		    mObj.outlet(mOutlet, possibleInt);
		    mBuf.setLength(0);
		    return;
		} catch (NumberFormatException nfe) {
		    // not an int - try float
		}
		
		try {
		    float possibleFloat = Float.valueOf(str).floatValue();
		    
		    mObj.outlet(mOutlet, possibleFloat);
		    mBuf.setLength(0);
		    return;
		} catch (NumberFormatException nfe) {
		    // not a float - it's a message of 0 args
		}
		
		mObj.outlet(mOutlet, str, Atom.emptyArray);
		
		mBuf.setLength(0);
		return;
	    } else {
		// it's a multi-token thang
		
		// grab the first token and analyze it
		String token = str.substring(0, spacePos);
		
		// if it's a number, consider sending a list
		try {
		    Integer.parseInt(token);
		    Float.valueOf(str).floatValue();
		    
		    Atom list[] = Atom.parse(token, false);
		    mObj.outlet(mOutlet, list);
		    
		    mBuf.setLength(0);
		    return;
		} catch (NumberFormatException nfe) {
		    // not a number - must be a message
		}
		
		mObj.outlet(mOutlet, token,
			    Atom.parse(str, true));
		
		mBuf.setLength(0);
		return;
	    }
	}
    }

    /**
     * Write an array of characters.
     */
    public void write(char[] cbuf) throws IOException {

	synchronized (mBuf) {
	    checkOpen();
	
	    int length = cbuf.length;
	    for (int i = 0; i < length; i++) {
		char ch = cbuf[i];
		if (ch == EOL) {
		    flush();
		} else {
		    mBuf.append(ch);
		}
	    }
	}
    }

    /**
     * Write a portion of an array of characters.
     */
    public void write(char[] cbuf, int off, int len) throws IOException {

	synchronized (mBuf) {
	    checkOpen();

	    for (int i = off; i < off + len - 1; i++) {
		char ch = cbuf[i];
		if (ch == EOL) {
		    flush();
		} else {
		    mBuf.append(ch);
		}
	    }
	}
    }

    /**
     * Write a single character.
     */
    public void write(int c) throws IOException {

	synchronized (mBuf) {
	    checkOpen();

	    char ch = (char)c;
	    if (ch == EOL) {
		flush();
	    } else {
		mBuf.append(ch);
	    }
	}
    }

    /**
     * Write a string.
     */
    public void write(String str) throws IOException {
	int len = str.length();
	char temp[] = new char[len];
	str.getChars(0, len, temp, 0);
	write(temp);
    }

    /**
     * Write a portion of a string - handle EOL.
     */
    public void write(String str, int off, int len) throws IOException {
	char temp[] = new char[len];
	str.getChars(off, off + len, temp, 0);
	write(temp);
    }

}


