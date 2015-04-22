//Copyright (c) 2001-3, Cycling '74.  All rights reserved.
//N.B. Any changes to this file require corresponding changes to Atom.h.

package com.cycling74.max;

import java.io.Serializable;
import java.util.StringTokenizer;

/**
 *	An element of a list or argument of a message.
 *	May contain an int, a float, or a String.
 *
 *	@author Herb Jellinek, Ben Nevile   
 */
public abstract class Atom implements Serializable, Comparable {

    /** 
	 * A zero-length Atom array.  This can come in handy. 
	 * Instead of generating a new one, it's faster to use this one.
	 */
    public static final Atom emptyArray[] = new Atom[0];	

    private int mType;		//the atom type, as defined in AtomDataTypes.java
	
	//code for the three atom types, for use by toDebugString
    private static final char INT_TYPE_CHAR = 'I';  
    private static final char FLOAT_TYPE_CHAR = 'F';
    private static final char STRING_TYPE_CHAR = 'S';

    //
    // Inner subclasses.
    //

    private static final class FloatAtom extends Atom {

		private float mValue = 0.0f;
		
		private FloatAtom(float value) {
			super(AtomDataTypes.FLOAT);
			mValue = value;
		}
		
		public boolean equals(Object o) {
			return (o instanceof FloatAtom) && 
						(((FloatAtom)o).getFloat() == getFloat());
		}
		
		public int compareTo(Object o) {
			if (o instanceof FloatAtom) {
				if (this.equals(o))
					return 0;
				else if (((FloatAtom)o).getFloat() > this.getFloat())
					return -1;
				else return 1;
			} else if (o instanceof IntAtom) {
				float f = (float)((IntAtom)o).getInt();
				if (f == this.getFloat())
					return 0;
				else if (f > this.getFloat())
					return -1;
				else return 1;
			} else if (o instanceof StringAtom) {
				return 1;
			} else if (o == null)
				throw new NullPointerException();
			else
				throw new ClassCastException();
				
		}

		public int hashCode() {
			return Float.floatToIntBits(mValue);
		}
				
		public boolean isFloat() {
			return true;
		}

		public float getFloat() {
			return mValue;
		}
		
		public byte toByte() {
			return (byte)mValue;
		}
		
		public short toShort() {
			return (short)mValue;
		}
				
		public int toInt() {
			return (int)mValue;
		}
		
		public long toLong() {
			return (long)mValue;
		}
		
		public float toFloat() {
			return mValue;
		}
		
		public double toDouble() {
			return (double)mValue;
		}
		
		public String toString() {
			return Float.toString(mValue);
		}

		public char toChar() {
			return (char)mValue;
		}
		
		public boolean toBoolean() {
			return (mValue != 0.0f);
		}
		
		
		public Object toObject() {    
			return new Float(mValue);
		}
    }

    private static final class IntAtom extends Atom {

		private int mValue = 0;
		
		private IntAtom(int value) {
			super(AtomDataTypes.LONG);
			mValue = value;
		}
		
		public boolean equals(Object o) {
			return (o instanceof IntAtom) &&
			(((IntAtom)o).getInt() == getInt());
		}

		public int compareTo(Object o) {
			if (o instanceof IntAtom) {
				if (this.equals(o))
					return 0;
				if (((IntAtom)o).getInt() > this.getInt())
					return -1;
				else return 1;
			} else if (o instanceof FloatAtom) {
				float f = ((FloatAtom)o).getFloat();
				if (f == (float)(this.getInt())) {
					return 0;
				} else if (f > (float)(this.getInt())) {
					return -1;
				} else  //if (f < (float)(this.getInt())) 
					return 1;
			} else if (o instanceof StringAtom) {
				return 1;
			} else if (o == null)
				throw new NullPointerException();
			else
				throw new ClassCastException();
		}

		public int hashCode() {
			return mValue;
		}

		public boolean isInt() {
			return true;
		}
		
		public int getInt() {
			return mValue;
		}
		
		public byte toByte() {
			return (byte)mValue;
		}
		
		public short toShort() {
			return (short)mValue;
		}
		
		public int toInt() {
			return mValue;
		}
		
		public long toLong() {
			return (long)mValue;
		}
		
		public float toFloat() {
			return (float)mValue;
		}
		
		public double toDouble() {
			return (double)mValue;
		}
		
		public String toString() {
			return Integer.toString(mValue);
		}
		
		public char toChar() {
			return (char)mValue;
		}
		
		public boolean toBoolean() {
			return (mValue != 0);
		}
		
		public Object toObject() {
			return new Integer(mValue);
		}
    }

    private static final class StringAtom extends Atom {

		private String mValue = null;
		
		private StringAtom(String value) {
			super(AtomDataTypes.STRING);
			if (value == null) {
				throw new NullPointerException();
			}
			mValue = value;
		}
		
		public boolean equals(Object o) {
			return (o instanceof StringAtom) &&
					((StringAtom)o).getString().equals(getString());
		}
		
		public int compareTo(Object o) {
			if (o instanceof StringAtom) {
				return (this.getString().compareTo(((StringAtom)o).getString()));	
			} else if ((o instanceof IntAtom)||(o instanceof FloatAtom)) {
				return -1;
			} else if (o == null)
				throw new NullPointerException();
			else
				throw new ClassCastException();
		}

		public int hashCode() {
			return mValue.hashCode();
		}

		public String getString() {
			return mValue;
		}
		
		public boolean isString() {
			return true;
		}
		
		public String toString() {
			return getString();
		}
		
		public char toChar() {
			return mValue.charAt(0);
		}
		
		public boolean toBoolean() {
			return (!(mValue.equals("false")));
		}
		
		public Object toObject() {    
			return getString();
		}		
    }


	// private constructor -- must use one of the factory methods 
    private Atom(int type) {
		mType = type;
    }
	
	/**
     * Create and return an <code>Atom</code> containing an <code>int</code>.
	 * @param value the byte value to represent in the Atom
	 * @return an int Atom
     */
    public static Atom newAtom(byte value) {
		return new IntAtom((int)value);
    }

	/**
     * Create and return an <code>Atom</code> containing an <code>int</code>.
     * @param value the short value to represent in the Atom
	 * @return an int Atom
     */
    public static Atom newAtom(short value) {
		return new IntAtom((int)value);
    }

	/**
     * Create and return an <code>Atom</code> containing an <code>int</code>.
     * @param value the int value to represent in the Atom
	 * @return an int Atom
     */
    public static Atom newAtom(int value) {
		return new IntAtom(value);
    }

	/**
     * Create and return an <code>Atom</code> containing an <code>int</code>.
     * Be careful: if the input long is bigger than Integer.MAX_VALUE
     * the cast from long to int will overflow.
     * @param value the long value to represent in the Atom
	 * @return an int Atom
     */
    public static Atom newAtom(long value) {
		return new IntAtom((int)value);
    }

    /**
     * Create and return an <code>Atom</code> containing a <code>float</code>.
     * @param value the float value to represent in the Atom
	 * @return a float Atom
     */
    public static Atom newAtom(float value) {
		return new FloatAtom(value);
    }

    /**
     * Create and return an <code>Atom</code> containing a <code>float</code>.
     * Be careful:the reduction in numerical precision when casting from double to float
     * can result in a loss of information and/or overflow.
     * @param value the double value to represent in the Atom
	 * @return a float Atom
     */
    public static Atom newAtom(double value) {
		return new FloatAtom((float)value);
    }
    
    /**
     * Create and return an <code>Atom</code> containing a <code>String</code>
     * made up of a single character.
     * @param value the character to represent in the Atom
	 * @return a String Atom
     */
    public static Atom newAtom(char value) {
		return new StringAtom(new String(new char[] {value}));
    }

    /**
     * Create and return an <code>Atom</code> containing a <code>String</code>.
     * @param value the String to represent in the Atom
	 * @return a String Atom
     */
    public static Atom newAtom(String value) {
		return new StringAtom(value);
    }
    
    /**
     * Create and return an <code>Atom</code> containing a <code>int</code>
     * that is either 1 or 0 depending on the value of the argument.
     * @param value the boolean to represent in the Atom
	 * @return an int Atom whose value is either 0 or 1
     */
    public static Atom newAtom(boolean value) {
	    	if (value) 	return new IntAtom(1);
	    	else 	return new IntAtom(0);
    }
    
    /**
     * Create and return an <code>Atom</code> array 
     * which represents the array of <code>byte</code>s.
     * @param array the byte array to represent in the Atoms
	 * @return an integer Atom array
     */
    public static Atom[] newAtom(byte array[]) {
		int len = array.length;
		Atom a[] = new Atom[len];
		for (int i = 0; i < len; i++) {
			a[i] = Atom.newAtom(array[i]);
		}
		return a;
    }
    
    /**
     * Create and return an <code>Atom</code> array 
     * which represents the array of <code>short</code>s.
     * @param array the short array to represent in the Atoms
	 * @return an integer Atom array
     */
    public static Atom[] newAtom(short array[]) {
		int len = array.length;
		Atom a[] = new Atom[len];
		for (int i = 0; i < len; i++) {
			a[i] = Atom.newAtom(array[i]);
		}
		return a;
    }
    
    /**
     * Create and return an <code>Atom</code> array 
     * which represents the array of <code>int</code>s.
     * @param array the int array to represent in the Atoms
	 * @return an integer Atom array
     */
    public static Atom[] newAtom(int array[]) {
		int len = array.length;
		Atom a[] = new Atom[len];
		for (int i = 0; i < len; i++) {
			a[i] = Atom.newAtom(array[i]);
		}
		return a;
    }
    
    /**
     * Create and return an <code>Atom</code> array 
     * which represents the array of <code>long</code>s.
     * Be careful: if an input long is bigger than Integer.MAX_VALUE
     * the cast from long to int will overflow.
     * @param array the long array to represent in the Atoms
	 * @return an integer Atom array
     */
    public static Atom[] newAtom(long array[]) {
		int len = array.length;
		Atom a[] = new Atom[len];
		for (int i = 0; i < len; i++) {
			a[i] = Atom.newAtom(array[i]);
		}
		return a;
    }
    
    /**
     * Create and return an <code>Atom</code> array 
     * which represents the array of <code>float</code>s.
     * @param array the float array to represent in the Atoms
	 * @return a float Atom array
     */
    public static Atom[] newAtom(float array[]) {
		int len = array.length;
		Atom a[] = new Atom[len];
		for (int i = 0; i < len; i++) {
			a[i] = Atom.newAtom(array[i]);
		}
		return a;
    }
    
    /**
     * Create and return an <code>Atom</code> array 
     * which represents the array of <code>double</code>s.
     * Be careful:the reduction in numerical precision when casting from double to float
     * can result in a loss of information and/or overflow.
     * @param array the double array to represent in the Atoms
	 * @return a float Atom array
     */
    public static Atom[] newAtom(double array[]) {
		int len = array.length;
		Atom a[] = new Atom[len];
		for (int i = 0; i < len; i++) {
			a[i] = Atom.newAtom(array[i]);
		}
		return a;
    }
    
    /**
     * Create and return an <code>Atom</code> array 
     * which represents the array of <code>char</code>s as <code>String</code>s.
     * @param array the character array to represent in the Atoms
	 * @return a String Atom array
     */
    public static Atom[] newAtom(char array[]) {
		int len = array.length;
		Atom a[] = new Atom[len];
		for (int i = 0; i < len; i++) {
			a[i] = Atom.newAtom(array[i]);
		}
		return a;
    }

    /**
     * Create and return an <code>Atom</code> array 
     * which represents the array of <code>String</code>s.
     * @param array the String array to represent in the Atoms
	 * @return a String Atom array
     */
    public static Atom[] newAtom(String array[]) {
		int len = array.length;
		Atom a[] = new Atom[len];
		for (int i = 0; i < len; i++) {
			a[i] = Atom.newAtom(array[i]);
		}
		return a;
    }
    
    /**
     * Create and return an <code>Atom</code> array 
     * with the <code>String</code> represented in the
     * first <code>Atom</code> and the <code>args</code>
     * array after.  Useful for forwarding an anything
     * message to a method like list that takes an 
     * <code>Atom</code> array.
     * @param s the first element of the Atom array
     * @param args all the other elements of the Atom array
     * @return an Atom array
     */
    public static Atom[] newAtom(String s, Atom args[]) {
		Atom[] newAtomList = new Atom[args.length+1];
		newAtomList[0] = Atom.newAtom(s);
		for (int i = 1; i <= args.length; i++) 
			newAtomList[i] = args[i-1];
		return newAtomList;
    }

	/**
	 * Create and return an <code>Atom</code> array
	 * which represents the array of <code>boolean</code>s.
	 * @param array the boolean array to represent in the Atoms
	 * @return an int Atom array
	 */
    public static Atom[] newAtom(boolean array[]) {
    	int len = array.length;
    	Atom a[] = new Atom[len];
		for (int i = 0; i < len; i++) {
			a[i] = Atom.newAtom(array[i]);
		}
		return a;
    }
    

	//isInt, isFloat and isString are each only overridden in the subclass
	//that they correspond to.  eg, isInt is overridden in IntAtom to return
	//a true value but not in FloatAtom so in that case it will still use
	//the method below, which returns false.
    /**
     * Does this <code>Atom</code> represent an integer?
     * @return true if this contains an integer value (<code>int</code>)
     */
    public boolean isInt() {
		return false;
    }

    /**
     * Does this <code>Atom</code> represent a float?
     * @return true if this contains a <code>float</code>
     */
    public boolean isFloat() {
		return false;
    }

    /**
     * Does this <code>Atom</code> represent a String?
     * @return true if this contains a <code>String</code>
     */
    public boolean isString() {
		return false;
    }

    /**
     * Return the <code>float</code> contents of this <code>Atom</code>.
     *
     * @return the <code>float</code> value represented in the <code>Atom</code>
     * @exception java.lang.UnsupportedOperationException if the
     * <code>Atom</code> does not represent a <code>float</code>.
     */
    public float getFloat() {
		throw new UnsupportedOperationException("does not contain a float");
    }

    /**
     * Return the integer contents of this <code>Atom</code>.
     *
     * @return the <code>int</code> value represented in the <code>Atom</code>.
     * @exception java.lang.UnsupportedOperationException if the 
     * <code>Atom</code> does not represent an <code>int</code>.
     */
    public int getInt() {
		throw new UnsupportedOperationException("does not contain an int");
    }

    /**
     * Return the <code>String</code> contents of this <code>Atom</code>.
     *
     * @return the <code>String</code> value represented in the <code>Atom</code>.
     * @exception java.lang.UnsupportedOperationException if this <code>Atom</code>
     * does not represent a <code>String</code>.
     */
    public String getString() {
		throw new UnsupportedOperationException("does not contain a String");
    }

    /**
     * Return the contents of this <code>Atom</code> as a Java object.
     * If it contains an <code>int</code>, return an <code>Integer</code> object;
     * if it contains a <code>float</code>, return a <code>Float</code> object;
     * and if it contains a <code>String</code>, return a <code>String</code>.
     *
     * @return the contents of the <code>Atom</code> as an <code>Object</code>.
     */
    public abstract Object toObject();

    /**
     * Indicates whether some other object is "equal to" this one.
     *
     * @param o the other <code>Object</code> to test for equivalence
     *
     * @return <code>true</code> if and only if the other
     * <code>Object</code> is an <code>Atom</code> of the same 
     * content type (<code>float</code>, <code>int</code>, or <code>String</code>)
     * with equal contents.
     */
    public abstract boolean equals(Object o);

    /**
     * Returns a hash code for this <code>Atom</code>.
	 *
	 * For an <code>Atom</code> that represents an <code>int</code>, 
	 * the value returned is the <code>int</code>
	 * value itself.  For an <code>Atom</code> that represents a 
	 * <code>float</code> the result is the integer bit
	 * representation produced by the <code>Float.floatToIntBits(float)</code> 
	 * method.  For an <code>Atom</code> that represents a <code>String</code>
	 * the result is the integer bit
	 * representation produced by the {@link java.lang.String#hashCode()} method.
     *
     * @return  a hash code value for this object, calculated using an 
     *          algorithm appropriate to the <code>Atom</code>'s content type.
     */
    public abstract int hashCode();
	
  	/**
     * Reverses the elements of an <code>Atom</code> array 
	 * and returns the result.
	 *
     * @param atoms the array of <code>Atom</code>s to operate on
     *
     * @return a reversed copy of the <code>atoms</code> parameter
     */
	public static Atom[] reverse(Atom atoms[]) {
		for (int i=0;i<atoms.length/2;i++) {
			Atom temp = atoms[i];
			atoms[i] = atoms[atoms.length-i-1];
			atoms[atoms.length-i-1] = temp;
		}
		return atoms;
	}
	
  	/**
     * Rotates the elements of an <code>Atom</code> array 
	 * by <code>numberOfPlaces</code> and returns the result.
	 *
     * @param atoms the array of <code>Atom</code>s to operate on
     * @param numberOfPlaces the number of places to rotate the array
     * by (can be negative)
     *
     * @return a rotated copy of the <code>atoms</code> parameter
     */
	public static Atom[] rotate(Atom atoms[], int numberOfPlaces) {	
		int newStart = numberOfPlaces 
						- atoms.length*(numberOfPlaces/atoms.length);
		while (newStart < 0)
			newStart += atoms.length;
		Atom[] temp = new Atom[atoms.length];
		System.arraycopy(atoms, 0, temp, newStart, atoms.length - newStart);
		System.arraycopy(atoms, atoms.length-newStart, temp, 0, newStart);
		return temp;
	}
	
  	/**
     * Returns the intersection of two <code>Atom</code> arrays.
	 *
     * @param a an array of <code>Atom</code>s to operate on
     * @param b another array of <code>Atom</code>s to operate on
     *
     * @return an array that contains the intersection 
     * of <code>a</code> and <code>b</code> 
     */
	public static Atom[] intersection(Atom[] a, Atom[] b) {
		Atom[] temp = new Atom[a.length];
		int count = 0;
		for (int i=0;i<a.length;i++) 
			if ( 	(Atom.isIn(a[i], a, i-1, 0) == -1)
				&&	(Atom.isIn(a[i], b) != -1)) {
				temp[count] = a[i];
				count++;
			} 
		temp = Atom.removeLast(temp, a.length-count);
		return temp;
	}	
	
  	/**
     * Returns the union of two <code>Atom</code> arrays.
	 *
     * @param a an array of <code>Atom</code>s to operate on
     * @param b another array of <code>Atom</code>s to operate on
     *
     * @return an array that contains the union 
     * of <code>a</code> and <code>b</code> 
     */
	public static Atom[] union(Atom[] a, Atom[] b) {
		Atom[] temp = new Atom[a.length+b.length];
		System.arraycopy(a,0,temp,0,a.length);
		System.arraycopy(b,0,temp,a.length,b.length);
		
		int i = 0;
		while (i<temp.length) {
			if (Atom.isIn(temp[i], temp, i-1, 0) != -1) 
				temp = Atom.removeOne(temp, i);
			else
				i++;
		}
		return temp;
	}	

	
 	/**
     * Is <code>item</code> in array <code>atoms</code>?
	 *
	 * @param item the <code>Atom</code> to search for
     * @param atoms the array of <code>Atom</code>s to search
     *
     * @return an integer that represents the position of the found item,
     * or -1 if the item was not found
     */
	public static int isIn(Atom item, Atom[] atoms) {
		return isIn(item, atoms, atoms.length-1, 0);
	}
	
 	/**
     * Is <code>item</code> in array <code>atoms</code>
     * between indices <code>highIdx</code> and <code>lowIdx</code>?
	 *
	 * @param item the <code>Atom</code> to search for
     * @param atoms the array of <code>Atom</code>s to search
     * @param highIdx the highest index of the array to check
     * @param lowIdx the lowest index of the array to check
     *
     * @return an integer that represents the position of the found item,
     * or -1 if the item was not found
     */
	public static int isIn(Atom item, Atom[] atoms, int highIdx, int lowIdx) {
		int found = -1;
		int i = highIdx;
		while ((i >= lowIdx)&&(found==-1)) {
			if (atoms[i].equals(item))
				found = i;
			i--;
		}
		return found;
	}

 	/**
     * Removes one element of an <code>Atom</code> array 
	 * and returns the result.
     * If <code>atoms</code> has length of zero, returns it unchanged.
	 *
     * @param atoms the array of <code>Atom</code>s to operate on
     * @param index the index of the element to remove
     *
     * @return a copy of the <code>atoms</code> parameter, with the
     * index element removed
     */	
	public static Atom[] removeOne(Atom atoms[], int index) {
		return removeSome(atoms, index, index);
	}
	
  	/**
     * Removes some elements of an <code>Atom</code> array 
	 * and returns the result.
     * If <code>atoms</code> has length of zero, returns it unchanged.
	 *
     * @param atoms the array of <code>Atom</code>s to operate on
     * @param first the index of the first element of the range to remove
     * @param last the index of the last element of the range to remove
     *
     * @return a copy of the <code>atoms</code> parameter, with all elements
     * between first and last (inclusive) removed
     */
    public static Atom[] removeSome(Atom atoms[], int first, int last) {
		int length = atoms.length;
		if (length == 0) {
			return atoms;
		}
		if ((first < 0)||(first > length-1)) {
			throw new IllegalArgumentException("first index out of range");
		} else if ((last < 0)||(last > length-1)) {
			throw new IllegalArgumentException("last index out of range");
		} else if (last < first) {
			throw new IllegalArgumentException("last index is smaller than first");
		} else if  ((first == 0)&&(last == length-1)) {
			return emptyArray;
		}
		Atom result[] = new Atom[length-(last-first+1)];
		System.arraycopy(atoms, 0, result, 0, first);
		System.arraycopy(atoms, last+1, result, first, length-last-1);
		return result;
    }
  
    /**
     * Removes the first element of an <code>Atom</code> array 
	 * and returns the result.
     * If <code>atoms</code> has length of zero, returns it unchanged.
	 * If <code>atoms</code> has a length of one, returns the
	 * {@link com.cycling74.max.Atom#emptyArray}.
     *
     * @param atoms the array of <code>Atom</code>s to operate on
     *
     * @return a copy of the <code>atoms</code> parameter, one element removed
	 * from the left
     */
    public static Atom[] removeFirst(Atom atoms[]) {
		return removeFirst(atoms, 1);
    }

    /**
     * Removes the first <code>howMany</code> elements of an <code>Atom</code>
     * array and returns the result.
     * If <code>atoms</code> has length of zero, returns
	 * it unchanged.
	 * If <code>atoms</code> has length of <code>howMany</code>, returns the
	 * {@link com.cycling74.max.Atom#emptyArray}.
     *
     * @param atoms the <code>Atom</code> array to operate on
     * @param howMany the number of elements to remove
     *
     * @return a copy of the <code>atoms</code> parameter, <code>howMany</code>
	 * elements removed from the left
     *
     * @exception IllegalArgumentException If howMany is a negative
     * amount or more than the array's length.
     */
    public static Atom[] removeFirst(Atom atoms[], int howMany) {
		if (howMany < 0) {
			throw new IllegalArgumentException("howMany must be >= 0");
		}
		int length = atoms.length;
		if (length == 0) {
			return atoms;
		} else if  (length == howMany) {
			return emptyArray;
		}
		if (howMany > length) {
			throw new IllegalArgumentException("howMany > length");
		}
		Atom result[] = new Atom[length-howMany];
		System.arraycopy(atoms, howMany, result, 0, length - howMany);
		return result;
    }
    
 
    /**
     * Removes the last element of an <code>Atom</code> array 
	 * and returns the result.
     * If <code>atoms</code> has length of zero, returns it unchanged.
	 * If <code>atoms</code> has a length of one, returns the
	 * {@link com.cycling74.max.Atom#emptyArray}.
     *
     * @param atoms the array of <code>Atom</code>s to operate on
     *
     * @return a copy of the <code>atoms</code> parameter, one element removed
	 * from the left
     */
    public static Atom[] removeLast(Atom atoms[]) {
		return removeLast(atoms, 1);
    }

    /**
     * Removes the last <code>howMany</code> elements of an <code>Atom</code>
     * array and returns the result.
     * If <code>atoms</code> has length of zero, returns
	 * it unchanged.
	 * If <code>atoms</code> has length of <code>howMany</code>, returns the
	 * {@link com.cycling74.max.Atom#emptyArray}.
     *
     * @param atoms the <code>Atom</code> array to operate on
     * @param howMany the number of elements to remove
     *
     * @return a copy of the <code>atoms</code> parameter, <code>howMany</code>
	 * elements removed from the left
     *
     * @exception IllegalArgumentException If howMany is a negative
     * amount or more than the array's length.
     */
    public static Atom[] removeLast(Atom atoms[], int howMany) {
		if (howMany < 0) {
			throw new IllegalArgumentException("howMany must be >= 0");
		}
		int length = atoms.length;
		if (length == 0) {
			return atoms;
		} else if  (length == howMany) {
			return emptyArray;
		}
		if (howMany > length) {
			throw new IllegalArgumentException("howMany > length");
		}
		Atom result[] = new Atom[length-howMany];
		System.arraycopy(atoms, 0, result, 0, length - howMany);
		return result;
    }
     
    /**
     * Convert an <code>Atom</code> array to a single <code>String</code>.  The 
	 * resulting <code>String</code> contains the result of calling 
	 * <code>toString</code> on all of
     * the <code>Atom</code>s in the array, separated by a space character.
     *
     * @param array the <code>Atom</code> array to convert to one <code>String</code>.
     * @return a <code>String</code> containing representations of all of
     * the <code>Atom</code>s in the array
     */
    public static String toOneString(Atom array[]) {
		int len = array.length;
		StringBuffer sb = new StringBuffer(4 * len);
		for (int i = 0; i < len; i++) {
			sb.append(array[i].toString());
			if (i != (len - 1)) {
			sb.append(" ");
			}
		}
		return sb.toString();
    }    

    /**
     * Convert an <code>Atom</code> array to an array of <code>String</code>s.  
     *
     * @return a <code>String</code> array containing representations of all of
     * the <code>Atom</code>s in the array
     *
     * @param array the <code>Atom</code> array to convert to a <code>String</code> array.
     */
    public static String[] toString(Atom array[]) {
		int len = array.length;
		String s[] = new String[len];
		for (int i = 0; i < len; i++) {
			s[i]=array[i].toString();
		}
		return s;
    }    
    
    /**
     * Converts an <code>Atom</code> to a <code>byte</code>, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents a <code>float</code>, its fractional portion 
     * will be truncated, then the resulting <code>int</code> will be cast to a <code>byte</code>.  
     * If the Atom represents a <code>String</code>, 0 will be returned.
     *
     * @return a <code>byte</code> that represents the value of the <code>Atom</code>
     */
    public byte toByte() {
    	//method is overridden in subclasses
    	return 0;
    }    
    
    /**
     * Converts an <code>Atom</code> array to a <code>byte</code> array, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents a <code>float</code>, its fractional portion 
     * will be truncated and the resulting <code>int</code> cast to a <code>byte</code>.  
     * If the Atom represents a <code>String</code>, 0 will be returned.
     * @param array the <code>Atom</code> array to convert to a <code>byte</code> array.
     * @return an <code>int</code> array that represents the values of the 
     * <code>Atom</code> array, coerced to the integer type if necessary
     */
    public static byte[] toByte(Atom array[]) {
		int len = array.length;
		byte bytes[] = new byte[len];
		for (int i = 0; i < len; i++) {
			bytes[i]=array[i].toByte();
		}
		return bytes;
    }    

    /**
     * Converts an <code>Atom</code> to a <code>short</code>, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents a <code>float</code>, its fractional portion 
     * will be truncated, then the resulting <code>int</code> will be cast to a <code>short</code>.  
     * If the Atom represents a <code>String</code>, 0 will be returned.
     *
     * @return a <code>short</code> that represents the value of the <code>Atom</code>
     */
    public short toShort() {
    	//method is overridden in subclasses
    	return 0;
    }    
    
    /**
     * Converts an <code>Atom</code> array to a <code>short</code> array, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents a <code>float</code>, its fractional portion 
     * will be truncated and the resulting <code>int</code> cast to a <code>short</code>.  
     * If the Atom represents a <code>String</code>, 0 will be returned.
     * @param array the <code>Atom</code> array to convert to a <code>short</code> array.
     * @return a <code>short</code> array that represents the values of the 
     * <code>Atom</code> array
     */
    public static short[] toShort(Atom array[]) {
		int len = array.length;
		short shorts[] = new short[len];
		for (int i = 0; i < len; i++) {
			shorts[i]=array[i].toShort();
		}
		return shorts;
    }    

    /**
     * Convert an <code>Atom</code> to an <code>int</code>, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents a <code>float</code>, its fractional portion 
     * will be truncated.  If the Atom represents a <code>String</code>, 0
     * will be returned.
     * @return an <code>int</code> that represents the value of the <code>Atom</code>, coerced
     * to the integer type if necessary
     */
    public int toInt() {
    	//method is overridden in subclasses
    	return 0;
    }    
    
    /**
     * Convert an <code>Atom</code> array to an <code>int</code> array, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents a <code>float</code>, its fractional portion 
     * will be truncated.  If the Atom represents a <code>String</code>, 0
     * will be returned.
     * @param array the <code>Atom</code> array to convert to an <code>int</code> array.
     * @return an <code>int</code> array that represents the values of the 
     * <code>Atom</code> array, coerced to the integer type if necessary
     */
    public static int[] toInt(Atom array[]) {
		int len = array.length;
		int ints[] = new int[len];
		for (int i = 0; i < len; i++) {
			ints[i]=array[i].toInt();
		}
		return ints;
    }    

    /**
     * Convert an <code>Atom</code> to a <code>long</code>, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents a <code>float</code>, its fractional portion 
     * will be truncated.  If the Atom represents a <code>String</code>, 0
     * will be returned.
     * @return a <code>long</code> that represents the value of the <code>Atom</code>, coerced
     * to the long type if necessary
     */
    public long toLong() {
    	//method is overridden in subclasses
    	return 0;
    }    
    
    /**
     * Convert an <code>Atom</code> array to a <code>long</code> array, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents a <code>float</code>, its fractional portion 
     * will be truncated.  If the Atom represents a <code>String</code>, 0
     * will be returned.
     * @param array the <code>Atom</code> array to convert to a <code>long</code> array.
     * @return a <code>long</code> array that represents the values of the 
     * <code>Atom</code> array
     */
    public static long[] toLong(Atom array[]) {
		int len = array.length;
		long longs[] = new long[len];
		for (int i = 0; i < len; i++) {
			longs[i]=array[i].toLong();
		}
		return longs;
    }    

    /**
     * Convert an <code>Atom</code> to a <code>float</code>, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents an <code>int</code>, it is converted to a float.  
     * If the Atom represents a <code>String</code>, 0.0f will be returned.
     *
     * @return a <code>float</code> that represents the value of the <code>Atom</code>, coerced
     * to the float type if necessary
     */
    public float toFloat() {
    	//method is overridden in subclasses
    	return 0.0f;
    } 
    
    /**
     * Convert an <code>Atom</code> array to a <code>float</code> array, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents an <code>int</code>, it is converted to a float.  
     * If the Atom represents a <code>String</code>, 0.0f will be returned.
     * @param array the <code>Atom</code> array to convert to a <code>float</code> array.
     * @return a <code>float</code> array that represents the values of the 
     * <code>Atom</code> array, coerced to the <code>float</code> type if necessary
     */
    public static float[] toFloat(Atom array[]) {
		int len = array.length;
		float floats[] = new float[len];
		for (int i = 0; i < len; i++) {
			floats[i]=array[i].toFloat();
		}
		return floats;
    } 


   /**
     * Convert an <code>Atom</code> to a <code>double</code>, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents an <code>int</code>, it is converted to a double.  
     * If the Atom represents a <code>String</code>, 0.0 will be returned.
     *
     * @return a <code>double</code> that represents the value of the <code>Atom</code>, coerced
     * to the double type if necessary
     */
    public double toDouble() {
    	//method is overridden in subclasses
    	return 0.0;
    } 
    
    /**
     * Convert an <code>Atom</code> array to a <code>double</code> array, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents an <code>int</code>, it is converted to a double.  
     * If the Atom represents a <code>String</code>, 0.0 will be returned.
     * @param array the <code>Atom</code> array to convert to a <code>double</code> array.
     * @return a <code>double</code> array that represents the values of the 
     * <code>Atom</code> array, coerced to the <code>double</code> type if necessary
     */
    public static double[] toDouble(Atom array[]) {
		int len = array.length;
		double doubles[] = new double[len];
		for (int i = 0; i < len; i++) {
			doubles[i]=array[i].toDouble();
		}
		return doubles;
    } 


	/**
     * Convert an <code>Atom</code> to a <code>char</code>, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents an <code>int</code> or a <code>float</code>, 
     * it is cast to a <code>char</code>.
     * If the Atom represents a <code>String</code>, its first character is returned.
     *
     * @return a <code>char</code> that represents the value of the <code>Atom</code>
     */
    public char toChar() {
    	//method is overridden in subclasses
    	return 0;
    } 
    
    /**
     * Convert an <code>Atom</code> array to a <code>char</code> array, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents an <code>int</code> or a <code>float</code>, 
     * it is cast to a <code>char</code>.
     * If the Atom represents a <code>String</code>, its first character is returned.
     * @param array the <code>Atom</code> array to convert to a <code>char</code> array
     * @return a <code>char</code> array that represents the values of the 
     * <code>Atom</code> array
     */
    public static char[] toChar(Atom array[]) {
		int len = array.length;
		char chars[] = new char[len];
		for (int i = 0; i < len; i++) {
			chars[i]=array[i].toChar();
		}
		return chars;
    } 


	/**
     * Convert an <code>Atom</code> to a <code>boolean</code>, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents an <code>int</code>, true is returned for all non-zero values.
     * If the Atom represents a <code>float</code>, true is returned for all non-zero values.
     * If the Atom represents a <code>String</code>, true is returned for all values that
     * aren't "false".
     *
     * @return a <code>boolean</code> that represents the value of the <code>Atom</code>
     */
    public boolean toBoolean() {
    	//method is overridden in subclasses
    	return true;
    } 
    
    /**
     * Convert an <code>Atom</code> array to a <code>boolean</code> array, following the
     * Atom type coercion rules if necessary.  
     * If the Atom represents an <code>int</code>, true is returned for all non-zero values.
     * If the Atom represents a <code>float</code>, true is returned for all non-zero values.
     * If the Atom represents a <code>String</code>, true is returned for all values that
     * aren't "false".
     * @param array the <code>Atom</code> array to convert to a <code>boolean</code> array.
     * @return a <code>boolean</code> array that represents the values of the 
     * <code>Atom</code> array
     */
    public static boolean[] toBoolean(Atom array[]) {
		int len = array.length;
		boolean bools[] = new boolean[len];
		for (int i = 0; i < len; i++) {
			bools[i]=array[i].toBoolean();
		}
		return bools;
    } 
    
    /**
     * Parse a <code>String</code> as a sequence of tokens
     * and return an <code>Atom</code> array. 
     *
     * @param str the <code>String</code> to parse
     *
     * @return an <code>Atom</code> array corresponding to the tokens in
     * the <code>String</code>
     */
    public static Atom[] parse(String str) {
    	return parse(str, false);
    }

    /**
     * Parse a <code>String</code> as a sequence of tokens
     * and return an <code>Atom</code>
     * array. If <code>skipFirst</code> is true, do not include the first
     * <code>Atom</code> in the result.
     *
     * @param str the <code>String</code> to parse
     * @param skipFirst whether to include the first <code>Atom</code>
     * in the result
     *
     * @return an <code>Atom</code> array corresponding to the tokens in
     * the <code>String</code>
     */
    public static Atom[] parse(String str, boolean skipFirst) {
		StringTokenizer st = new StringTokenizer(str, " \t");
		int length = st.countTokens();
		if (skipFirst) {
			length -= 1;
			st.nextToken();
		}
		Atom result[] = new Atom[length];
		for (int i = 0; i < length; i++) {
			String token = st.nextToken();
			try {
			int pi = Integer.parseInt(token);
			result[i] = Atom.newAtom(pi);
			continue;
			} catch (NumberFormatException nfe) {
			}
			try {
			float pf = Float.valueOf(token).floatValue();
			result[i] = Atom.newAtom(pf);
			continue;
			} catch (NumberFormatException nfe) {
			}
			result[i] = Atom.newAtom(token);
		}
		return result;
    }

    // Return the type of the object contained in this Atom.  Result
	// corresponds to one of the elements of AtomDataTypes.
    int getType() {
		return mType;
    }

    /**
     * Converts an <code>Atom</code> array to a printed representation suitable
     * for debugging.
	 *
	 * So if givena three-element Atom array containing the 
	 * <code>int</code> 1, the <code>float</code> 3.14 and the 
	 * <code>String</code> "test" as input, the output would be 
	 * <code>Atom[3]={1:I}{3.14:F}{test:S}</code>.
     *
     * @param array an array of <code>Atom</code> objects
     *
     * @return a verbose <code>String</code> describing the elements
     * of <code>array</code>
     */
    public static String toDebugString(Atom array[]) {
		StringBuffer sb = new StringBuffer("Atom");
		sb.append("[");
		sb.append(array.length);
		sb.append("]=");
		for (int i = 0; i < array.length; i++) {
			Atom elt = array[i];
			char code;
			sb.append("{");
			sb.append(elt);
			sb.append(":");
			switch (elt.getType()) {
				case AtomDataTypes.LONG:
					code = INT_TYPE_CHAR;
					break;
				case AtomDataTypes.FLOAT:
					code = FLOAT_TYPE_CHAR;
					break;
				case AtomDataTypes.STRING:
					code = STRING_TYPE_CHAR;
					break;
				default:
					code = '?';
			}
			sb.append(code);
			sb.append("}");
		}
		return sb.toString();
    }
    
    /**
     * Determines if <code>Object o</code> is "bigger than" this 
	 * <code>Atom</code>.  <code>Atom</code>s that 
	 * represent <code>String</code>s are defined to be less than <code>
	 * int</code>s and <code>float</code>s - that is, if you make use of the
	 * {@link java.util.Arrays#sort(Object[])} method, <code>String</code>s
	 * will be sorted first.  Comparisons between <code>Atom</code>s that 
	 * represent <code>float</code>s and <code>int</code>s give the results  
	 * you would expect.
	 * 
     *
     * @param o the <code>Object</code> to compare with.
	 *
	 * @throws NullPointerException if the input is null
	 * @throws ClassCastException if something other than an Atom is passed
	 * in as an argument
	 * @return -1 if o is bigger than the called object, 0 if they're equal, 
	 * and 1 if o is smaller
     */
    public int compareTo(Object o) {
	    	//this is overridden in each of the three subclasses
	    	return 0;
    }
}
