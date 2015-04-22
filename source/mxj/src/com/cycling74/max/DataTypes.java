
/*
 * Copyright (c) 2001-2003, Cycling '74.  All rights reserved.
 */

package com.cycling74.max;

/**
 * This class contains constants describing the various data types
 * that MaxObjects can pass through their outlets and
 * inlets, and a method to make them readable.
 *
 * @author Herb Jellinek
 */

public final class DataTypes {

    /**
     * This inlet or outlet can pass integers.
     */
    public static final int INT = 1;

    /**
     * This inlet or outlet can pass floats.
     */
    public static final int FLOAT = 2;

    /**
     * This inlet or outlet can pass lists (arrays) of <tt>Atom</tt>s
     */
    public static final int LIST = 4;

    /**
     * This inlet or outlet can pass messages, meaning
     * a <tt>String</tt> plus an array of <tt>Atom</tt>s.
     */
    public static final int MESSAGE = 8;

    /**
     * How many bits wide is the widest valid type descriptor?
     * This is equal to (log2(highest type) + 1).
     */
    private static final int bitsNeeded = 4;

    /**
     * All things considered: this inlet or outlet can pass <tt>INT</tt>,
     * <tt>FLOAT</tt>, <tt>LIST</tt>, and/or <tt>MESSAGE</tt>.
     */
    public static final int ALL = (INT | FLOAT | LIST | MESSAGE);

    /**
     * All things considered - synonym for <tt>ALL</tt>.  This inlet or outlet
     * can pass <tt>INT</tt>, <tt>FLOAT</tt>, <tt>LIST</tt>,
     * and/or <tt>MESSAGE</tt>.
     */
    public static final int ANYTHING = ALL;

    /**
     * All the individual type names as an array, except <tt>ALL</tt> and
     * <tt>noTypesName</tt>
     */
    private static final String typeNames[] = { "int", "float",
						"list", "message" };

    /**
     * Each type mask.
     */
    private static final int typeMasks[] = { INT, FLOAT, LIST, MESSAGE };

    /**
     * The name of "no types."
     */
    private static final String noTypesName = "NONE";

    /**
     * The name of "all types."
     */
    private static final String allTypesName = "ALL";

    /**
     * Separator to use between names.
     */
    private static final char typeSep = ';';

    /**
     * You can't create onee of these.
     */
    private DataTypes() {
    }

    /**
     * Renders a type description as a name.
     *
     * @param typeDescr an integer type description code, a bit mask derived
     * from combinations of the constant values defined in this class.
     *
     * @return a human-readable description of the <tt>typeDescr</tt> argument
     */
    public static String toString(int typeDescr) {
	boolean seenOne = false;
	int masked = typeDescr & ALL;
	if (masked == ALL) {
	    return allTypesName;
	}
	if (masked == 0) {
	    return noTypesName;
	}
	StringBuffer sb = new StringBuffer(10);
	for (int i = 0; i < bitsNeeded; i++) {
	    if ((masked & typeMasks[i]) != 0) {
		if (seenOne) {
		    sb.append(typeSep);
		}
		sb.append(typeNames[i]);
		seenOne = true;
	    }
	}
	return sb.toString();
    }

    /* test method.
    public static void main(String args[]) {
	for (int i = 0; i < 16; i++) {
	    System.out.println(i+" "+getName(i));
	}
    }
    */
}
