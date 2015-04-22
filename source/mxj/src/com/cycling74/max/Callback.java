package com.cycling74.max;
import java.lang.reflect.*;

/**
 *
 * <code>Callback</code> is an implementation of the Executable interface.
 * One could use an instance of <code>Callback</code> as part of the constructor
 * for a <code>MaxClock</code> or <code>MaxQelem</code> as follows:
 * 
 * 
 * <br>
 *<PRE>
 * public class callbacktest extends MaxObject {
 *	
 *	private static final double DELAY_TIME = 500.;
 *	private Callback cb;
 *	private MaxClock cl;
 *	
 *	callbacktest() {
 *		cb = new Callback(this, "daBomb");
 *		cl = new MaxClock(cb);
 *	}
 *	
 *	public void bang() {
 *		cl.delay(DELAY_TIME);
 *	}
 *	
 *	private void daBomb() {
 *		post("bOOm!");
 *		cl.delay(DELAY_TIME);
 *	}
 *}
 *   </PRE>
 *   <br>
 * 
 * Using <code>Callback</code> it is possible to define a parameter list
 * so that something like <code>MaxClock</code> or <code>MaxQelem</code> can be made to 
 * call a method that takes parameters.  Furthermore, one can
 * alter the parameters at anytime.  For instance:
 * 
 * <PRE>
 public class Callbacktest extends MaxObject {
	private static final double DELAY_TIME = 500.;
	private Callback cb;
<code>Callback</code>e MaxClock cl;
	
	callbacktest() {
		cb = new Callback(this, "daBomb", true);
		cl = new MaxClock(cb);
	}
	
	public void bang() {
		cl.delay(DELAY_TIME);
	}
	
	private void daBomb(boolean b) {
		if (b)
			post(" is da bomb!");
		else
			post("not da bomb."):
		cb.setArgs(!b);
		cl.delay(DELAY_TIME);
	}
}
   </PRE>
 * 
 * created on 5-May-2004
 * @author Ben Nevile
 */
public class Callback implements Executable
{
	private static final String jerr = "java programming error: ";
	private Object obj = null;
	private String methodName = null;
	private Method meth = null;
	private Object[] args = null;
	
	/**
	 * creates a <code>Callback</code> object for an executing method with no arguments
	 * @param o object that contains the executing method 
	 * @param methodname name of the executing method
	 */
	public Callback(Object o, String methodname) {
		this(o, methodname, null, null);
	}
	
	/**
	 * creates a <code>Callback</code> object for an executing method with one int argument
	 * @param o object that contains the executing method
	 * @param methodname name of the executing method
	 * @param i the integer to pass as an argument to the executing method
	 */
	public Callback(Object o, String methodname, int i) {
		this(o, methodname, new Object[] {new Integer(i)}, 
				new Class[] {java.lang.Integer.TYPE});
	}
	
	/**
	 * creates a <code>Callback</code> object for an executing method with one float argument
	 * @param o object that contains the executing method
	 * @param methodname name of the executing method
	 * @param f the float to pass as an argument to the executing method
	 */
	public Callback(Object o, String methodname, float f) {
		this(o, methodname, new Object[] {new Float(f)}, 
				new Class[] {java.lang.Float.TYPE});
	}
	
	/**
	 * creates a <code>Callback</code> object for an executing method with one String argument
	 * @param o object that contains the executing method
	 * @param methodname name of the executing method
	 * @param s the String to pass as an argument to the executing method
	 */
	public Callback(Object o, String methodname, String s) {
		this(o, methodname, new Object[] {s});
	}	
	
	/**
	 * creates a <code>Callback</code> object for an executing method with one boolean argument
	 * @param o object that contains the executing method
	 * @param methodname name of the executing method
	 * @param b the boolean value to pass as an argument to the executing method
	 */
	public Callback(Object o, String methodname, boolean b) {
			this(o, methodname, new Object[] {(b?Boolean.TRUE:Boolean.FALSE)}, 
					new Class[] {java.lang.Boolean.TYPE});
	}
	
	/**
	 * creates a <code>Callback</code> object for an executing method with a 
	 * parameter list represented by the Object array argObjectArray.  
	 * Note that primitive types (<code>boolean</code>, <code>char</code>, 
	 * <code>byte</code>, <code>short</code>, <code>int</code>, <code>long</code>, 
	 * <code>float</code>, <code>double</code>)
	 * are not supported by this method - the corresponding wrapper objects 
	 * (<code>Boolean</code>, <code>Char</code>, <code>Byte</code>, <code>Short</code>, 
	 * <code>Integer</code>, <code>Long</code>, <code>Float</code>, <code>Double</code>) 
	 * must be used.
	 * To mix objects and primitive types, please refer to 
	 * <code>Callback(Object, String, Object[], Class[])</code>.
	 * @param o object that contains the executing method
	 * @param methodname name of the executing method
	 * @param argObjectArray the object array to pass as an argument to the executing method
	 */
	public Callback(Object o, String methodname, Object[] argObjectArray) {
		this(o, methodname, argObjectArray, toClasses(argObjectArray));
	}
	
	/**
	 * creates a <code>Callback</code> object for an executing method with 
	 * a parameter list represented by the Object array argObjectArray
	 * and with Class list argClassArray.
	 * By specifying the Class array explicitly, this method allows you to pass
	 * primitive types as arguments.  To enable one of the primitive types 
	 * (<code>boolean</code>, <code>char</code>, 
	 * <code>byte</code>, <code>short</code>, <code>int</code>, <code>long</code>, 
	 * <code>float</code>, <code>double</code>) as an
	 * element of the argument array, that element of argObjectArray
	 * must be set to the corresponding wrapper Object (<code>Boolean</code>, <code>Char</code>, <code>Byte</code>, <code>Short</code>, 
	 * <code>Integer</code>, <code>Long</code>, <code>Float</code>, <code>Double</code>)
	 *  with the value of the primitive type, and the element
	 * of the argClassArray must be set to the special corresponding Class object 
	 * that is set up to represent the primitive as an Class (<code>java.lang.Boolean.TYPE</code>,
	 * <code>java.lang.Character.TYPE</code>, <code>java.lang.Byte.TYPE</code>, 
	 * <code>java.lang.Short.TYPE</code>, <code>java.lang.Integer.TYPE</code>,
	 * <code>java.lang.Long.TYPE</code>, <code>java.lang.Float.TYPE</code>, 
	 * <code>java.lang.Double.TYPE</code>).  
	 * <br>
	 * The class below shows an example that uses this constructor to define
	 * a <code>Callback</code> that executes a method that takes an int and an <code>Atom</code> array as 
	 * parameters.  
	 * <br>
	 * <pre>
	 * public class CallbackExample extends MaxObject {
	 * 
	 * 	private static final DELAY_TIME = 500.;
	 * 	Callback cb;
	 *  MaxClock ck;	
	 * 
	 * 	CallbackExample(Atom[] initArgs) {
	 * 		cb = new Callback(this, 
	 * 					"doThis", 
	 * 					new Object[] {new Integer(1), initArgs},
	 * 					new Class[] {java.lang.Integer.TYPE, initArgs.getClass()});
	 * 		ck = new MaxClock(cb);
	 * 	}
	 * 
	 * 	public void bang() {
	 * 		ck.delay(DELAY_TIME);
	 * 	}
	 * 
	 * 	private method doThis(int i, Atom[] a) {
	 * 		// do stuff
	 * 	}
	 * 
	 * }
	 * </pre>
	 * @param o object that contains the executing method
	 * @param methodname name of the executing method
	 * @param argObjectArray the object array to pass as an argument to the executing method
	 * @param argClassArray the class array that defines the classes of the argument
	 */
	public Callback(Object o, String methodname, Object[] argObjectArray, Class[] argClassArray) {
		obj = o;
		this.methodName = methodname;
		if (argObjectArray != null) {
			setArgs(argObjectArray);
			meth = resolveMethod(o, methodname, argClassArray);
		} else {
			meth = resolveMethod(o, methodname, null);
		}
		if (meth != null)
			if (!meth.isAccessible()) //not a public method?
				meth.setAccessible(true);  //it don't matter.
	}
	
	/**
	 * Set the argument for an executing method that takes an integer. 
	 * Calling this method will cause the <code>Callback</code> to fail if the 
	 * <code>Callback</code> was not constructed with a single int parameter!
	 * @param i the new value to pass the executing method
	 */
	public void setArgs(int i) {
		args = new Object[] {new Integer(i)};
	}
	
	/**
	 * Set the argument for an executing method that takes a float. 
	 * Calling this method will cause the <code>Callback</code> to fail if the 
	 * <code>Callback</code> was not constructed with a single float parameter!
	 * @param f the new value to pass the executing method
	 */
	public void setArgs(float f) {
		args = new Object[] {new Float(f)};
	}
	
	/**
	 * Set the argument for an executing method that takes a boolean. 
	 * Calling this method will cause the <code>Callback</code> to fail if the 
	 * <code>Callback</code> was not constructed with a single boolean parameter!
	 * @param b the new value to pass the executing method
	 */
	public void setArgs(boolean b) {
		if (b)
			args = new Object[] {Boolean.TRUE};
		else
			args = new Object[] {Boolean.FALSE};
	}
	
	/**
	 * Set the argument for an executing method that takes a String. 
	 * Calling this method will cause the <code>Callback</code> to fail if the 
	 * <code>Callback</code> was not constructed with a single String parameter!
	 * @param s the new value to pass the executing method
	 */
	public void setArgs(String s) {
		args = new Object[] {s};
	}
	
	/**
	 * Set the argument array for an executing method.
	 * No check is done on the format of the array passed in, 
	 * so if the array does not match the signature of the executing
	 * method, the <code>Callback</code> will fail!
	 * @param a the new array to pass as an argument to the executing method
	 */
	public void setArgs(Object[] a) {
		args = new Object[a.length];
		System.arraycopy(a, 0, args, 0, a.length);
	}
	
	//creates a Class array from an Object array
	private static Class[] toClasses(Object[] o) {
		Class c[] = new Class[o.length];
		for (int i=0;i<o.length;i++) 
			c[i] = o[i].getClass();
		return c;
	}
	
	//returns the Method that matches the name and signature. 
	//if it does not exist, returns null.
	private static Method resolveMethod(Object o, String name, Class[] c) {
		Method m;
		try {
			m = o.getClass().getDeclaredMethod(name, c);
			return m;
		} catch (NoSuchMethodException e) {
			MaxObject.error(jerr+"bad method / signature for Callback");
			return null;
		}
	}
	
	/** 
	 * executes the method associated with the <code>Callback</code>.
	 * @see com.cycling74.max.Executable#execute()
	 */
	public void execute() {
		try {
			meth.invoke(obj, args);
		} catch (InvocationTargetException e) {
			MaxObject.error(jerr+"Callback InvocationTargetException");
		} catch (IllegalAccessException e) {
			MaxObject.error(jerr+"Callback IllegalAccessException");
		}
	}
	
	/**
	 * Gets the current parameters that will be passed to the executing method.
	 * Since the return type is an Objcet array, the data has to be cast
	 * back to its proper Class to be useful.  For instance, if you know that
	 * your parameter list is made up of an int, a float, and a <code>MaxClock</code>, you could
	 * do something like this:
	 * <br>
	 * <pre>
	 * public manipulateArgs(Callback cb) {
	 * 	Object[] paramObjs = c.getArgs;
	 * 	if (paramObjs.length > 0) {
	 * 		int i = ((Integer)paramObjs[0]).intValue();
	 * 		float f = ((Float)paramObjs[1]).floatValue();
	 * 		MaxClock cl = (MaxClock)paramObjs[2];
	 *		// now do stuff with the variables...
	 * 	}
	 * }
	 * </pre>
	 * <br>
	 * 
	 * 
	 * @return the current parameter Object array
	 */
	public Object[] getArgs() {
		return args;
	}
	
	/**
	 * Gets the Object that contains the executing method.
	 * 
	 * @return the Object that contains the executing method
	 */
	public Object getObject() {
		return obj;
	}
	
	/**
	 * Gets the executing method.
	 * 
	 * @return the executing method
	 */
	public Method getMethod() {
		return meth;
	}
	
	/**
	 * Gets the name of the executing method.
	 * 
	 * @return a String that contains the name of the executing method
	 */
	public String getMethodName() {
		return methodName;
	}
}