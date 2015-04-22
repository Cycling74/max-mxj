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



/**
 * Array-related utilities.
 * 
 * @author Herb Jellinek
 */
public class ArrayUtilities {

    /**
     * You can't instantiate this class.
     */
    private ArrayUtilities() {
    }

    /**
     * Copy array <tt>src</tt> and return the copy.
     */
    public static int[] arrayCopy(int src[]) {
	int len = src.length;
	int dest[] = new int[len];
	System.arraycopy(src, 0, dest, 0, len);
	return dest;
    }

    /**
     * Check if the array of Object contains null. 
     * @return true if the array contains at least one null element, 
     * <tt>false</tt> otherwise
     */
    public static boolean containsNull(Object array[]) {
	int len = array.length;
	for (int i = 0; i < len; i++) {
	    if (array[i] == null) {
		return true;
	    }
	}
	return false;
    }
}
