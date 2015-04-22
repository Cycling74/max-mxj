
/*
 * Copyright (c) 2001-2003, Cycling '74.  All rights reserved.
 */

package com.cycling74.max;

/**
 * The various data types Max can hold in an Atom object.
 * An Atom is a constituent of a list.
 * <br>
 * N.B. Any changes to this file require corresponding changes to Atom.h.
 * @author Herb Jellinek
 */
interface AtomDataTypes {

    /**
     * An integer.  This is named LONG for historical reasons.
     */
    public static final int LONG = 1;

    /**
     * A float.
     */
    public static final int FLOAT = 2;

    /**
     * A String.
     */
    public static final int STRING = 3;
}
