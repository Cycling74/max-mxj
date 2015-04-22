/*
 * Copyright (c) 2003, Cycling '74
 * 
 * This software is the confidential and proprietary information of 
 * Cycling '74 ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Cycling '74.
 * 
 * CYCLING '74 MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
 * CYCLING '74 SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 *
 */

package com.cycling74.util;

import java.io.*;

/**
 * Input/Output utility class.
 * 
 * @author Herb Jellinek
 */
public class IOUtilities {

    /**
     * You cannot instantiate this class.
     */
    private IOUtilities() {
    }

    /**
     * Read and return the first line of the <tt>InputStream</tt>.
     * The first line is defined as the characters from the current
     * stream position through end-of-file or the first character for
     * which <tt>isEof</tt> returns true, whichever comes first.
     *
     * <br>
     *
     * Closing the <tt>InputStream</tt> is left to the caller.
     */
    public static String readFirstLine(InputStream is) {
	StringBuffer sb = new StringBuffer();

	try {
	    while (true) {
		int ch = is.read();
		if ((ch == -1) || isEof((char)ch)) {
		    break;
		}
		sb.append((char)ch);
	    }
	} catch (IOException ie) {
	    // not a whole lot to do but log it
	    ie.printStackTrace(System.err);
	}

	return sb.toString();
    }

    /**
     * Is the argument an end-of-line character, meaning a carriage return
     * or linefeed?
     */
    public static boolean isEof(char c) {
	return (c == '\n' || c == '\r');
    }

}
