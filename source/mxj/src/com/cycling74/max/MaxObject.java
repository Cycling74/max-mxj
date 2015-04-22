
/*
 * Copyright (c) 2002, Cycling '74.  All rights reserved.
 */

package com.cycling74.max;

import com.cycling74.util.ArrayUtilities;
import com.cycling74.io.PostStream;
import com.cycling74.io.ErrorStream;
import com.cycling74.max.AttributeInfo;
import com.cycling74.mxjedit.MXJEditor;

import java.util.Vector;
import java.util.HashMap;
import java.util.Arrays;
import java.lang.reflect.*;
import javax.swing.*;
import java.io.*;

import java.util.jar.*;
import java.util.Enumeration;
import java.net.URLDecoder;

/**
 * Base class for a Java Max object.  Subclass this directly or
 * indirectly to create your own Java objects for use in Max.
 *
 * @author Herb Jellinek, Topher LaFata, Ben Nevile
 */
public abstract class MaxObject {

    // Note: All native methods begin with 'do'.
    //
    // Note: Max calls the following <code>MaxObject</code> methods:
    // 
    // getNumOutlets - called once when mxj object is created
    // getOutletType - called once when mxj object is created
    // getOutletAssist - can be called by Max at any time
    //
    // getNumInlets - called once when mxj object is created
    // getInletType - called once when mxj object is created
    // getInletAssist - can be called by Max at any time
    //

  	//Currently not in use!!
    private static final String HELPMESSAGE           = "help!";
  
 
    //used for auto invocation of methods from message name
    private static final Class  ATOM_ARRAY_CLASS        = (new Atom[0]).getClass();
    private static final Class  BOOL_ARRAY_CLASS        = (new boolean[0]).getClass();
    private static final Class  BYTE_ARRAY_CLASS        = (new byte[0]).getClass();
    private static final Class  CHAR_ARRAY_CLASS        = (new char[0]).getClass();
    private static final Class  SHORT_ARRAY_CLASS       = (new short[0]).getClass();
    private static final Class  INT_ARRAY_CLASS         = (new int[0]).getClass();
    private static final Class  LONG_ARRAY_CLASS        = (new long[0]).getClass();
    private static final Class  FLOAT_ARRAY_CLASS       = (new float[0]).getClass();
    private static final Class  DOUBLE_ARRAY_CLASS      = (new double[0]).getClass();
    private static final Class  STRING_ARRAY_CLASS      = (new String[0]).getClass();
	private static final Class  OBJECT_ARRAY_CLASS      = (new Object[0]).getClass();
    
    
    private static final Class  GIMME_PARAM_TYPE[]    =  new Class[] { ATOM_ARRAY_CLASS};
    private static final Class  ANYTHING_PARAM_TYPES[]    =  new Class[] {(new String()).getClass(), ATOM_ARRAY_CLASS};
  	private static final String ANYTHING_METHOD_NAME  = "anything";
  	private static final String GIMME_STRING          = "__GIMME__";
    private static final Class STRING_CLASS           = (new String()).getClass();

    
   /*global streams for use by all */
    private static final PostStream sPostStream   = PostStream.getPostStream();
    private static final ErrorStream sErrorStream = ErrorStream.getErrorStream();   
  
    /* MXJEditor associated with this object. see viewsource */
    private MXJEditor _source_editor = null;
   	/*
    * Sometimes we wish to do visual feedback when things could take
    * a while. For instance viewsource sometimes takes a while to pop
    * up the editor. So we provide visual feedback by cycling through
    * box colors.
    * We use this _flash_watch mechanism so a spawned thread
    * and its parent can agree on when to stop the box changing colors.
	*/
	private Object _flash_watch  = null;
	private Object WATCHME = new Object();  
	
    static 
    {	
		//redirect standard error and out to max console
		System.setErr(ErrorStream.getErrorStream());
		System.setOut(PostStream.getPostStream());
    }

   
  
    /**
     * A convenience constant; use it in a call to <tt>declareInlets</tt>
     * to declare that an object has no inlets.
     * @see #declareInlets(int[])
     */
    public static final int NO_INLETS[] = new int[0];

    /**
     * A convenience constant; use it in a call to <tt>declareOutlets</tt>
     * to declare that an object has no outlets.
     * @see #declareOutlets(int[])
     */
    public static final int NO_OUTLETS[] = new int[0];

    /**
     * A convenience constant; use this empty <code>String</code>
     * array instead of creating a new one.
     */
    public static final String[] EMPTY_STRING_ARRAY = new String[0];
    

    /**
     * A pointer to the C Max object that points to us.
     */
    private long mPeer = 0L;

    /**
     * The name of this instance. Used by MaxContext
     */
    private String mName = null;

    /**
     * Our inlets, defined as an array of DataTypes values.
     */
    private int mInlets[] = NO_INLETS;

    /**
     * Our outlets, defined as an array of DataTypes values.
     */
    private int mOutlets[] = NO_OUTLETS;
    
    private String[] mInletAssist  = EMPTY_STRING_ARRAY;
    
    private String[] mOutletAssist = EMPTY_STRING_ARRAY;
    
	private boolean mCreateInfoOutlet = true;
	
	private Vector _attributes = new Vector();

    /**
     * Build a Max object.  You must declare the number and types of
     * the inlets here using <tt>declareInlets</tt> and
     * <tt>declareOutlets</tt>.
     * @param args An array of Atom objects representing the typed-in 
     * to this object.  The zero-th item is always the name of the Java class
     * being instantiated.
     */
     
   /**
     * This allows subclasses to avoid calling super(args) in their 
     * constructors.
     */
    protected MaxObject() 
    {
		setName(getClass().getName());
 		MaxContext.register(this);
 		determineIfInletMethodsExist();
 		declareIO(1,1);
    }
 
    //
    // Notification that our peer has been deleted (disa-peer-ed?)
    //
    
    /**
     * Called from Max: Our C mxj peer has been deleted.  Tell MaxContext
     * to forget about us, and asynchronously notify the object itself.
     */
    private void mxjObjectWasDeleted() {
		
		MaxContext.unregister(this);
		notifyDeleted();
   
		// help the garbage collector
   		mName          = null;
    	mInlets        = null;
    	mOutlets       = null;
    	mInletAssist   = null;
    	mOutletAssist  = null;
		_attributes    = null;
		_flash_watch   = null;
    	WATCHME        = null;
    	_source_editor = null;
    }

    /**
     * Notification that the corresponding mxj object peer was deleted.  By
     * default, this method does nothing.  Subclasses are free to override this to
     * do anything they like.
     */
    protected void notifyDeleted() {}

    
    //
    // Naming
    //

    /**
     * Set the name of this instance used by MaxContext.
     * @param name the new name.
     */
    public void setName(String name) {
		mName = name;
    }

    /**
     * Get the MaxContext name of this instance.
     * @return a <code>String</code> that contains the instance's name. 
     */
    public String getName() {
		return mName;
    }

 
    //
    // Access to our context
    //

    /**
     * Get the context in which we are running.
     * @return the current <code>MaxContext</code>.
     */
    public static MaxContext getContext() {
		return MaxContext.getSingleton();
    }

    /**
     * Post a message to the Max console.
     * @param message the <code>String</code> to post in the Max console.
     */
    public static void post(String message) {
	//	byte msgBytes[] = message.getBytes();
	//	doPost(msgBytes);
    	MaxSystem.post(message);
    }

    /**
     * Post an error message to the Max console.
     * @param message the <code>String</code> to post as an error to the Max console.
     */
    public static void error(String message) {
//		byte msgBytes[] = message.getBytes();
//	    doError(msgBytes);
    	MaxSystem.error(message);
    }

    /**
     * Put up an error window.  Extremely annoying.  
     * Should probably never be used.
     * I'm not even sure why we support it.
     * @param message the text to display in the error window.
     */
    public static void ouch(String message) {
    	MaxSystem.ouch(message);
    }

    /**
     * Get the global <tt>PostStream</tt>.
     * @return the global <code>PostStream</code>.
     */
    public static PostStream getPostStream() {
		return sPostStream;
    }

    /**
     * Display an exception or other <tt>Throwable</tt> in a convenient way.
     * Prints out the exception information in the Max console.
     * @param t the exception to display.
     */
    public static void showException(Throwable t) {
		showException(null, t);
    }

    /**
     * Display an exception or other <tt>Throwable</tt> in a
     * convenient way with an accompanying message.
     * @param message the accompanying message.
     * @param t the exception to display.
     */
    public static void showException(String message, Throwable t) {
		PostStream ps = getPostStream();
		if (message != null) {
	    	ps.println(message);
	}
		t.printStackTrace(ps);
		ps.flush();
    }

    /**
     * Get the global <tt>ErrorStream</tt>.
     * @return the global <code>ErrorStream</code>.
     */
    public static ErrorStream getErrorStream() {
		return sErrorStream;
    }

    /**
     * Post a message to the Max console.
     * Internal method.
     */
    private static final native void doPost(byte message[]);

    /**
     * Post an error message to the Max console.  Max objects can subscribe to
     * error messages.
     * Internal method.
     */
    private static final native void doError(byte message[]);

    /**
     * Put up a Max error window.
     * Internal method.
     */
    private static final native void doOuch(byte message[]);

    //
    // Splash screen
    //

	
	/* gets pointer to t_patcher *thispatcher in mxj peer */
	private final native long _get_parent_patcher();
	/* gets pointer to t_box *thisbox in mxj peer */
	private final native long _get_max_box();
	
	
	
	/**
	 * Gets the parent <code>MaxPatcher</code>.
	 * @return the parent <code>MaxPatcher</code>
	 */
	public MaxPatcher getParentPatcher() {
		
		if(_parent_patcher == null){
			_parent_patcher = new MaxPatcher(_get_parent_patcher());
		}
			return _parent_patcher;
	}
	private MaxPatcher _parent_patcher = null;
	

	private MaxBox _max_box = null;
	/**
	 * Gets the MaxBox peer of this mxj instance.
	 *
	 * @return the <code>MaxBox</code> for this MaxObject
	 */
	public MaxBox getMaxBox() {
		if(_max_box == null)
			_max_box = new MaxBox(getParentPatcher(),_get_max_box());
		return _max_box;
	}

    //
    // Inlet/outlet declaration methods
    //
    /**
     * Declare typed inlets.  Typed inlets are more
     * efficient than untyped (ie, <code>DataTypes.ALL</code>).  To have
     * any effect, this method must be called in your class's constructor.
     * For example, the following call ensures that an object of 
     * this class will have an int inlet and a float inlet.
     * <pre>
     * declareInlets(DataTypes.INT, DataTypes.FLOAT);
     * </pre>
     * @param types array of ints that represent the type of data that 
     * will be accepted in the corresponding inlet.  
     * @see com.cycling74.max#DataTypes
     */
    protected void declareInlets(int types[]) {
		mInlets = ArrayUtilities.arrayCopy(types);
    }  

    /**
     * Declare typed outlets.  Typed inlets are more
     * efficient than untyped (ie, <code>DataTypes.ALL</code>).  To have
     * any effect, this method must be called in your class's constructor.
     * For example, the following call ensures that an object of 
     * this class will have an int outlet and a float outlet.
     * <pre>
     * declareOutlets(DataTypes.INT, DataTypes.FLOAT);
     * </pre>
     * @param types array of ints that represent the type of data that 
     * will be sent out the corresponding inlet.  
     * @see com.cycling74.max#DataTypes
     */
    protected void declareOutlets(int types[]) {
		mOutlets = ArrayUtilities.arrayCopy(types);
    }
    
    /**
     * Declare untyped inlets and outlets.  All inlets and outlets
     * are declared to be of type <code>DataTypes.ALL</code>.
     * @param ins the number of untyped inlets.
     * @param outs the number of untyped outlets.
     */
    protected void declareIO(int ins, int outs) {
		int[] temp = new int[ins];
		for (int i=0;i<ins;i++)
			temp[i]=DataTypes.ALL;
		mInlets = ArrayUtilities.arrayCopy(temp);
		temp = new int[outs];
		for (int i=0;i<outs;i++)
			temp[i]=DataTypes.ALL;
		mOutlets = ArrayUtilities.arrayCopy(temp);
	}
    
    
    /**
     * Declare typed inlets and outlets.  The two input
     * <code>String</code>s determine the number and type of inlets and outlets.
     * For instance, the following call declares that an object will have
     * two <tt>int</tt> inlets, a <tt>float</tt> inlet, then another <tt>int</tt>
     * inlet, and one <tt>LIST</tt> outlet.
     * <br>
     * <pre>
     * declareTypedIO("iifi","L");
     * </pre>
     * <br> 
     * F or f -> <code>DataTypes.FLOAT</code>
     * <br>
     * I or i -> <code>DataTypes.INT</code>
     * <br>
     * M or m -> <code>DataTypes.MESSAGE</code>
     * <br>
     * L or l -> <code>DataTypes.LIST</code>
     * <br>
     * other -> <code>DataTypes.ALL</code>
     * @param ins a <code>String</code> whose length and content determines the
     * number and type of the inlets.
     * @param outs a <code>String</code> whose length and content determines the
     * number and type of the outlets.
     */
    protected void declareTypedIO(String ins, String outs) {
    	int[] temp = new int[ins.length()];
		for (int i=0;i<ins.length();i++)
			temp[i] = convertCharToType(ins.charAt(i));
		mInlets = ArrayUtilities.arrayCopy(temp);
		
		temp = new int[outs.length()];
		for (int i=0;i<outs.length();i++)
			temp[i] = convertCharToType(outs.charAt(i));
		mOutlets = ArrayUtilities.arrayCopy(temp);
    }
    
    private int convertCharToType(char c) {
    	int i = DataTypes.ALL;
    	switch (c) {
    		case 'F':
    		case 'f':
    			i=DataTypes.FLOAT;
    			break;
    		case 'I':
    		case 'i':
    			i=DataTypes.INT;
    			break;
    		case 'M':
    		case 'm':
    			i=DataTypes.MESSAGE;
    			break;
    		case 'L':
    		case 'l':
    			i=DataTypes.LIST;
    			break;
    	}
    	return i;
    }
    
    /**
     * Set the inlet assistance strings.
     * @param messages an array of <code>String</code>s that will be displayed 
     * as assistance when the mouse is held over the corresponding inlet.
     */
    protected void setInletAssist(String[] messages)
    {
    	mInletAssist = messages;
    }
    
    
    /**
     * Set one inlet assistance string.
     * @param index the index of the inlet to set
     * @param message a <code>String</code> that will be displayed 
     * as assistance when the mouse is held over the corresponding inlet.
     */
    protected void setInletAssist(int index, String message) {
    	if (mInletAssist.length < index+1) {
    		String[] temp = new String[index+1];
    		System.arraycopy(mInletAssist, 0, temp, 0, mInletAssist.length);
    		temp[index] = message;
    		mInletAssist = temp;
    	} else 
    		mInletAssist[index] = message;
    }
    
    
    /**
     * Set the outlet assistance strings.
     * @param messages an array of <code>String</code>s that will be displayed 
     * as assistance when the mouse is held over the corresponding outlet.
     */
    protected void setOutletAssist(String[] messages)
    {
    	mOutletAssist = messages;
    }
    
    /**
     * Set one outlet assistance string.
     * @param index the index of the outlet to set
     * @param message a <code>String</code> that will be displayed 
     * as assistance when the mouse is held over the corresponding outlet.
     */
    protected void setOutletAssist(int index, String message) {
		if (mOutletAssist.length < index+1) {
			String[] temp = new String[index+1];
    		System.arraycopy(mOutletAssist, 0, temp, 0, mOutletAssist.length);
    		temp[index] = message;
    		mOutletAssist = temp;
    	} else 
    		mOutletAssist[index] = message;
    }
    
    private int[] getOutlets()
    {
    	return mOutlets;
    }
    
    private int[] getInlets()
    {
    	return mInlets;
    }
    
    /**
     * Determine whether or not an info outlet is created.  
     * By default an extra outlet is appended to the end of the outlets
     * requested in a class's constructor.  This is the info outlet, and
     * it is used to report information from attributes.  If the class uses
     * no attributes, or the designer of the class wishes to handle the 
     * getting of the attribute values manually, the info outlet may not be
     * necessary.  To get rid of it, call <code>createInfoOutlet(false)</code>
     * in the class's constructor. This method is only valid in the constructor. 
     * @param b boolean that determines whether or not the info outlet will be created.
     */
    protected void createInfoOutlet(boolean b)
    {
    	mCreateInfoOutlet = b;
    }
    
    /**
     * Returns the index of the info outlet.
     * @return the index of the info outlet.  Returns -1 if no info outlet exists.
     */
    public int getInfoIdx()
    {
    	if(mCreateInfoOutlet)
    		return mOutlets.length;
    	else {
    		return -1;
    	}
    }

    /**
     * Return the number of inlets.  
     * @return the number of inlets.
     */
    //this method is called by max..NOT ANYMORE. I AM PRETTY SURE -tml
    public int getNumInlets() {
		return mInlets.length;
    }

    /**
     * Returns an inlet type.
     * @param idx the index of the inlet to query
     * @return the type of the queried inlet
     *@see com.cycling74.max#DataTypes
     */
    //this method is called by max..NOT ANYMORE. I AM PRETTY SURE -tml
    public int getInletType(int idx) {
		return mInlets[idx];
    }

    /**
     * Returns the number of outlets.  
     * @return the number of outlets.
     */
    //this method is called by max..NOT ANYMORE. I AM PRETTY SURE -tml
    public int getNumOutlets() {
    		return mOutlets.length;//infor outlet is serviced in maxjava.c
    }

    /**
     * Returns an outlet type.
     * @param idx the index of the outlet to query
     * @return the type of the queried outlet
     * @see com.cycling74.max#DataTypes
     */
    //this method is called by max..NOT ANYMORE. I AM PRETTY SURE -tml
    public int getOutletType(int idx) {
		return mOutlets[idx];
    }

    //
    // Inlet methods.
    //

    /**
     * Describes what inlet <tt>inletIdx</tt> does.
     * <br>
     * Called by Max: returns an assist string for inlet <tt>inletIdx</tt>.
     * Should never be called for an inlet that doesn't exist!
     * <br>
     * <i>This implementation is pretty dumb.
     * One should normally use the setInletAssist methods to provide
     * inlet assistence to the user. 
     * This method can be overridden by subclasses if some special behavior is required.</i> 
     * @param inletIdx the index of the inlet for 
     * which assistance has been requested
     * @return a <code>String</code> that describes what the inlet does
     */
    public String getInletAssist(int inletIdx) 
    {
		//if(mInletAssist.length == mInlets.length)
		if(inletIdx < mInletAssist.length)
			return mInletAssist[inletIdx];	
		else
		{
		
			StringBuffer sb = new StringBuffer();
			sb.append(getClass().getName());
			sb.append(" inlet ");
			sb.append(inletIdx);
			sb.append(": type is ");
			if (inletIdx > getNumInlets()) {
	    		sb.append("OUT OF RANGE"); // should never happen
			} else {
	    		sb.append(DataTypes.toString(getInletType(inletIdx)));
			}
		return sb.toString();
    	}
    }

    //// default input methods and fall through logic
    // I've promised to implement this in C!  -bbn
    private boolean defaultInt = true,
					defaultFloat = true,
					defaultAtomList = true,
					defaultFloatList = true,
					defaultIntList = true,
					defaultAnything = true;
    
    private void determineIfInletMethodsExist() {
    	Method[] m = getAllMethodsButMaxObject(this);
    	for (int i=0;i<m.length;i++) {
    		String name = m[i].getName();
    		if (name.equals("inlet")) {
    			Class[] c = m[i].getParameterTypes();
    			if (c.length==1) {
    				String cname = c[0].getName();
    				if (cname.equals("int"))
    					defaultInt = false;
    				else if (cname.equals("float"))
    					defaultFloat = false;
    			}
    		}
    		else if (name.equals("list")) 
    		{
    			Class[] c = m[i].getParameterTypes();
    			if (c.length==1) {
    				String cname = c[0].getName();
    				if (cname.equals("[F"))
    					defaultFloatList = false;
    				else if (cname.equals("[I"))
    					defaultIntList = false;
    				else if (cname.equals("[Lcom.cycling74.max.Atom;"))
    					defaultAtomList = false;
    			}
				defaultAtomList = false;
    		}
   			else if (name.equals("anything")) {
    			defaultAnything = false;
    		}
    	}
    }
    
   /**
    * Called by Max: inlet has received a bang value.
    * Override this method in your subclass of MaxObject
    * if you want it to do something useful. By default
    * it does nothing.
    * @see #getInlet
    */
    protected void bang() {
    	post(getClass().getName()+" doesn't understand bang");
    }
    
    /**
     * Called by Max: inlet has received an <code>int</code> value.
     * Override this method in your subclass of <code>MaxObject</code>
     * if you want it to do something specific in response to an integer message.
     * If you do not override it, but you have defined your own
     * <code>inlet(float)</code> method, this default method
     * will cast the incoming <code>int</code> to <code>float</code> and
     * call <code>inlet(float)</code>.
     * If your subclass does not have either <code>inlet(int)</code> or
     * <code>inlet(float)</code>, this default method will call
     * <code>list(Atom[])</code> if it exists.  If none of those methods 
     * exist it will call <code>anything(String, Atom[])</code> with 
     * a message of <code>"int"</code> and a single-element in the <code>Atom</code>
     * array with the value of the incoming <code>int</code>.
     * To determine which inlet an int message arrived at in a multi inlet MaxObject
     * call getInlet(). This will always contain the index of the inlet which
     * last received a message.
     * @param value the <code>int</code> value that the object has received
     * @see #getInlet
     */
    protected void inlet(int value) {
    	if (!defaultFloat) {
    		inlet((float)value);
    		return;
    	}
    	else if (!defaultAtomList) {
    		list(new Atom[] {Atom.newAtom(value)});
    		return;
    	}
    	else {
    		anything("int", new Atom[] {Atom.newAtom(value)});
    		return;
    	}
    }

    /**
     * Called by Max: inlet has received a <code>float</code> value.
     * Override this method in your subclass of <code>MaxObject</code>
     * if you want it to do something specific.
     * If you do not override it, but you have defined your own
     * <code>inlet(int)</code> method, this default method
     * will cast the incoming <code>float</code> to <code>int</code> and
     * call <code>inlet(float)</code>.
     * If your subclass does not have either <code>inlet(float)</code> or
     * <code>inlet(int)</code>, this default method will call
     * <code>list(Atom[])</code> if it exists.  If none of those methods 
     * exist it will call <code>anything(String, Atom[])</code> with 
     * a message of <code>"float"</code> and a single-element in the <code>Atom</code>
     * array with the value of the incoming <code>float</code>.
     * To determine which inlet a float message arrived at in a multi inlet MaxObject
     * call getInlet(). This will always contain the index of the inlet which
     * last received a message.
     * @param value the <code>float</code> value that the object has received
     * @see #getInlet
     */
    protected void inlet(float value) {
    	if (!defaultInt) {
    		inlet((int)value);
    		return;
    	}
    	else if (!defaultAtomList) {
    		list(new Atom[] {Atom.newAtom(value)});
    		return;
    	}
    	else {
    		anything("float", new Atom[] {Atom.newAtom(value)});
    		return;
    	}
    }

    /**
     * Called by Max: inlet has received a list.
     * Override this method in your subclass of <code>MaxObject</code>
     * if you want it to do something specific.
     * If you do not override it, but you have defined your own
     * <code>anything(String, Atom[])</code> method, this default method
     * will pass the array to <code>anything</code> with 
     * a message of <code>"list"</code>.  If you have not defined your own
     * <code>anything</code> method, this default method attempts to map the
     * first element of the list to a matching <code>inlet</code> method.  For example,
     * if the first Atom represents an <code>int</code> it will call 
     * <code>inlet(int)</code> if you have it defined.  
     * @param atomArray the array of <code>Atom</code>s received by the object
     */
    protected void list(Atom atomArray[]) {
    	if (!defaultAnything) {
    		anything("list", atomArray);
    		return;
    	}
    	else if ((atomArray[0].getType() == DataTypes.INT) && (!defaultInt)) {
    		inlet(atomArray[0].getInt());
    		return;
    	}
    	else if ((atomArray[0].getType() == DataTypes.FLOAT) && (!defaultFloat)) {
    		inlet(atomArray[0].getFloat());
    		return;
    	}
    	else {
    		error(getClass().getName()+" doesn't understand list");
    		return;
    	}
    }

    /**
     * Called by Max: inlet has received a list of floats.
     * This method is faster than list(Atom[]).
     * Override this method in your subclass of <code>MaxObject</code>
     * if you want it to do something specific.
     * If you do not override it, this default method will pass the array to 
     * <code>list(Atom[])</code>.
     * 
     * @param f the array of <code>float</code>s received by the object.
     */
    protected void list(float f[])
    {
   		list(Atom.newAtom(f));
    }
    
    /**
     * Called by Max: inlet has received a list of ints.
     * This method is faster than list(Atom[]).
     * Override this method in your subclass of <code>MaxObject</code>
     * if you want it to do something specific.
     * If you do not override it, 
     * this default method will pass the array to <code>list(Atom[])</code>.
     * 
     * @param i the array of <code>int</code>s received by the object.
     */
    protected void list(int i[])
    {
    	list(Atom.newAtom(i));
    }
    
    

    /**
     * Called by Max: inlet has received a message.
     * Override this method in your subclass of <code>MaxObject</code>
     * if you want it to do something specific.
     * If there is a public method whose name and signature 
     * is identical to the incoming message,
     * this method will not be called.
     * 
     * @param message the incoming message
     * @param args the <code>Atom</code> arguments attached to the incoming message
     */
    protected void anything(String message, Atom args[]) {
    	if (message.equals("list")) {
    		list(args);
    		return;
    	}
   		error(getClass().getName()+" doesn't understand "+message);
    }
     
	/**
	 * This method returns the native filesystem path of the class file or jar archive 
	 * from which this class was loaded.
     * @return absolute native file system path of the class file or jar archive
	 * containing the bytecode for this class.
     */
     public String getCodeSourcePath()
	 {
       	try{
			java.net.URL url = this.getClass().getProtectionDomain().getCodeSource().getLocation();     
			String dirname = url.getFile();
			
			if(dirname.endsWith(".jar") || dirname.endsWith(".zip"))
			{
				if((new File(dirname)).exists())
					return dirname;
				else
					return null;
			}
			else
			{
				String name =  this.getClass().getName();
				String native_fs_name = name.replace('.',File.separatorChar);
				
				if(dirname.charAt(dirname.length() -1) != File.separatorChar)
					dirname = dirname + String.valueOf(File.separatorChar);
				
				String ret = dirname+native_fs_name+".class";
				if((new File(ret)).exists())
					return ret;
				else
					return null;
			}
					
		}catch(Exception e)
		{
			return null;
		}
	 }
	 
	 public void postCodePath()
	 {
		String loc = getCodeSourcePath();
		if(loc != null)
		{
			post("loc is : "+loc);
		}
	 }
    /**
     * Brings up the source code for the java class in an editor.
     * A convenient way to edit, save, and compile your classes.
     * Currently viewing the source of classes residing in jar files
     * is not supported. Source code is searched for at the exact same 
     * level in the filesystem as the class file is residing. For instance,
     * if you have a class com/boo/ya/Goober.class somewhere in the classpath
     * it will attempt to find the file com/boo/ya/Goober.java. If the source
     * file is not found at this location mxj will attempt to decompile the class file using
     * JODE (http://jode.sourceforge.net) and display its output in the editor.
     * This may not always be an accurate represenation of the original source.
     * Class files decompiled from a jar are displayed in the editor but are non-editable.
     * To edit these files you must first save them as a different file. This has to
     * do with the difficulty of modifying jar file contents in place.
     */
    public void viewsource()
    {
        String name = this.getClass().getName();
		String slashname = null;
		String native_fs_name = null;
    	String dirname = null; 
		//package . to /
		slashname = name.replace('.','/');
		native_fs_name = name.replace('.',File.separatorChar);
    	//this finds a file on the filesystem from a class object..not need here

       	
    	java.net.URL url = this.getClass().getProtectionDomain().getCodeSource().getLocation();        	
		dirname = new File(url.getFile()).getAbsolutePath();
	
    	if(dirname.charAt(dirname.length() -1) != File.separatorChar)
    		dirname = dirname + String.valueOf(File.separatorChar);
    		
    	String abspath = dirname+native_fs_name+".java";

    	if(url.toString().endsWith(".jar"))
    	{
    		try{
    			_viewsourcejar(slashname,URLDecoder.decode(url.getFile(),"UTF-8"));
    		}catch(UnsupportedEncodingException esee)
    		{
    			System.err.println("(mxj) problem decoding jar URL. Unable to decompile.");
    		}
    		return;
    	}
    	  	
    	final File f = new File(abspath);
    	if(!f.exists())
    	{
 			_flash_watch = WATCHME;
 			_flash_me();			
    		//We need to create the java file!
    		System.out.println("(mxj) Unable to locate "+abspath+". Decompiling with JODE");
    		try{
    			BufferedWriter bw = new BufferedWriter(new FileWriter(f)); 
    			MXJDecompiler.getInstance().decompile(name,MaxSystem.getClassPath(),bw);
    		}catch(IOException e)
    		{
				f.delete();
    			System.err.println("(mxj) Unable to decompile "+name);
    			System.err.println("message: "+e.getMessage());
    			_flash_watch = null;
    			return;
    		}
    		_flash_watch = null;	
    	}	
    	 	 		
    	if(_source_editor != null)
    	{
    		_flash_watch = WATCHME;
 			_flash_me();
    		_source_editor.setBufferFromFile(f);
    		_source_editor.setVisible(true);
    		_flash_watch = null;
    	}
    	else
    	{
    		SwingUtilities.invokeLater
			(
				new Runnable()
					{
						public void run()
						{
							try{
								
								_flash_watch = WATCHME;
								_flash_me();
								_source_editor = new MXJEditor();
								_source_editor.setBufferFromFile(f);
    							_source_editor.setVisible(true);							
								_flash_watch = null;
								
								}catch(Exception e)
								{
									_flash_watch = null;
									e.printStackTrace();
								}
						}
					}
			);
    		
    	}			
  	}

	private void _viewsourcejar(String slashname,String jarfile)
	{
		try
		{
			JarFile jar = new JarFile(new File(jarfile));
			for (Enumeration entries = jar.entries(); entries.hasMoreElements(); )
			{
				JarEntry entry = (JarEntry) entries.nextElement();

				if(entry.getName().equals(slashname+".java"))//we found source file in the jar
				{ 
					StringBuffer sb = new StringBuffer(1024);
    				byte[] buffer = new byte[1024];
        			int    bytesRead;
    				InputStream entryStream = jar.getInputStream(entry);
            		while ((bytesRead = entryStream.read(buffer)) != -1) 
            			sb.append(new String(buffer,0,bytesRead));
    				final String s = sb.toString();
    				final String dummyfilename = new String("!READONLY!:"+jar.getName()+"!/"+entry.getName());
    				SwingUtilities.invokeLater
					(
						new Runnable()
						{
							public void run()
							{
					
									_flash_watch = WATCHME;
									_flash_me();
									File f = new File(dummyfilename);
									_source_editor = new MXJEditor();
									_source_editor.setBuffer(s);
									_source_editor.setCurrentFile(f);
									_source_editor.setEditable(false);//this has the side effect of disabling the compile menu in setMode as well as forcing save as.. in save case
									_source_editor.setMode(MXJEditor.MODE_JAVA);
									_source_editor.setVisible(true);							
									_flash_watch = null;
							}
						}
					);
    				return;//we will return here if we found it
    			}
    		}
    		
    		//didn't find source in jar if we got to here...decompile
    		
    		StringWriter sw = new StringWriter(2048);
    		
    		_flash_watch = WATCHME;
    		_flash_me();
    		MXJDecompiler.getInstance().decompile(slashname.replaceAll("/","."),MaxSystem.getClassPath(),sw);
    		_flash_watch = null;
    		
    		sw.flush();
    		final String s = sw.toString();
    		final String dummyfilename = new String("!READONLY!:"+jar.getName()+"!/"+slashname+".java");
    		SwingUtilities.invokeLater
					(
						new Runnable()
						{
							public void run()
							{
								_flash_watch = WATCHME;
								_flash_me();
								File f = new File(dummyfilename);
								_source_editor = new MXJEditor();
								_source_editor.setBuffer(s);
								_source_editor.setCurrentFile(f);
								_source_editor.setEditable(false);//this has the side effect of disabling the compile menu in setMode as well as forcing save as.. in save case
								_source_editor.setMode(MXJEditor.MODE_JAVA);
    							_source_editor.setVisible(true);							
								_flash_watch = null;
							}
						}
					);
    		
    		
    	}catch(Exception e)
    		{
    			_flash_watch = null;
    			System.err.println("error viewing source from jarfile "+jarfile);
    			e.printStackTrace();
    		}
	
	}

	private void _flash_me()
	{
		/*
		removed flashing for now -jkc
		final MaxBox b = this.getMaxBox();
		Thread t = new Thread(new Runnable(){
			public void run()
			{
				final int cci = b.getColorIndex();
				while(_flash_watch != null)
				{
					MaxSystem.deferLow(new Executable(){
						public void execute()
						{ 
							b.setColorIndex(b.getColorIndex() + 10);

						}
					});	
					try{
						Thread.sleep(75);
					}catch(Exception e){}
				}

					MaxSystem.deferLow(new Executable(){
						public void execute()
						{
							b.setColorIndex(cci);
						}
					});	

			}		
		});
		t.start();
		*/
	}	

    /**
     * Returns the inlet of the last received message.  Eg,
     * <pre>
 		public inlet(int i) 
 		{
 			int index = getInlet();
 			post("int received in inlet "+index);
 		}
 		</pre> 
     * @return the inlet of the last received message.
     */
    protected native int getInlet();

    //
    // Outlet methods
    //

    /**
     * Describes what outlet <tt>outletIdx</tt> does.
     * <br>
     * Called by Max: returns an assist string for outlet <tt>outletIdx</tt>.
     * Should never be called for an outlet that doesn't exist!
     * <br>
     * <i>This implementation is pretty dumb. 
     * One should normally use the setOutletAssist methods to provide
     * outlet assistence to the user. 
     * This method can be overridden by subclasses if some special behavior is required.</i>
     * @param outletIdx the index of the outlet for 
     * which assistance has been requested
     * @return a <code>String</code> that describes what the outlet does
     */
    public String getOutletAssist(int outletIdx) 
    {
		//if(mOutletAssist.length == mOutlets.length)
		if(outletIdx < mOutletAssist.length)
		{
			return mOutletAssist[outletIdx];
		}
		else
		{
			StringBuffer sb = new StringBuffer();
			sb.append(getClass().getName());
			sb.append(" outlet ");
			sb.append(outletIdx);
			sb.append(": type is ");
			if (outletIdx > getNumOutlets())
			 {
		    	sb.append("OUT OF RANGE"); // should never happen
			} else 
			{
		    	sb.append(DataTypes.toString(getOutletType(outletIdx)));
			}
		return sb.toString();
    	}
    }

/*EXPERIMENTAL ...toph....
Implementation of a threadsafe mechanism that allows access to
the last atom/atoms outlet from an object indexed by outlet number. This is
sort of a ghetto way to make the functions of MaxObjects available programatically
to other MaxObjects in combination with attributes.
*/

/* I want to talk to bbn and jkc about this stuff before actually doing anything..
maybe it is stupid.....toph

private HashMap _last_output = new HashMap(32);


private void update_outlet_stack(int outletIdx, Atom[] output)
{
	String key  = Thread.currentThread().getName();
	Atom[][] vals = null;
	if((vals = (Atom[][])_last_output.get(key)) == null)
	{
		//support for the first 16 outlets
		vals = new Atom[16][];
		_last_output.put(key,new Atom[16][]);		
	}
	if(idx < 16)
		vals[idx] = output;
}

private Atom[] getOutput(int outletIdx)
{
	String key  = Thread.currentThread().getName();
	Atom[][] vals = null;
	if((vals = (Atom[][])_last_output.get(key)) != null && outletIdx < 16)
		return vals[idx];
	else
		return null;
}

END EXPERIMENTAL............ */

    /**
     * Sends a bang.
     * @param outletIdx the outlet index of the outlet to send the bang from.
     * @return true if successful.
     */
//EXPERIMENTAL	 
//	 private static final Atom[] BANG_AA_MESS = new Atom[]{Atom.newAtom("bang")};
    public final boolean outletBang(int outletIdx) 
	{
//		update_outlet_stack(outletIdx,BANG_AA_MESS);
		return doOutletBang(outletIdx);
    }


    /**
     * Sends an <code>int</code>.
     * @param outletIdx outlet number to send from 
     * @param value <code>int</code> to send
     * @return true if successful.
     */
//EXPERIMENTAL
//	private static final Atom[] INT_ELE_AA = new Atom[1];
    public final boolean outlet(int outletIdx, int value) 
	{
//		INT_ELE_AA[0] = Atom.newAtom(value);
//		update_outlet_stack(outletIdx,INT_ELE_AA);
	    return doOutlet(outletIdx, value);
    }

    /**
     * Sends a <code>byte</code>.
     * @param outletIdx outlet number to send from 
     * @param value <code>byte</code> to send
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, byte value) {
    	return outlet(outletIdx, (int)value);
    }

	 /**
	  * Sends a <code>short</code>.
	  * @param outletIdx outlet number to send from 
	  * @param value <code>short</code> to send
	  * @return true if successful.
	  */
    public final boolean outlet(int outletIdx, short value) {
    	return outlet(outletIdx, (int)value);
    }

    /**
     * Sends a <code>long</code>.
     * Warning, information can be lost in the conversion from <code>long</code>
     * to <code>int</code>!
     * @param outletIdx outlet number to send from 
     * @param value <code>long</code> to send
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, long value) {
    	return outlet(outletIdx, (int)value);
    }


    /**
     * Sends a <code>float</code>.
     * @param outletIdx outlet number to send from 
     * @param value <code>float</code> to send
     * @return true if successful.
     */
     

//EXPERIMENTAL
//	private static final Atom[] INT_ELE_AA = new Atom[1];
    public final boolean outlet(int outletIdx, float value) 
	{
//		FLT_ELE_AA[0] = Atom.newAtom(value);
//		update_outlet_stack(outletIdx,FLT_ELE_AA);
	    return doOutlet(outletIdx, value);
    }

    /**
     * Sends a <code>double</code>.
     * Warning, since the Max float atom type is a 32 bit floating point number
     * information can be lost in the conversion from a java <code>double</code>
     * to a native <code>float</code>!
     * @param outletIdx outlet number to send from 
     * @param value <code>double</code> to send
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, double value)
    {
    	return outlet(outletIdx, (float)value);
    }
    
    /**
     * Sends a message with no arguments.  This is equivalent to
     * <code>outlet(outletIdx, message, Atom.emptyArray)</code>.
     * 
     * @param outletIdx outlet number to send from
     * @param message message to send
     *
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, String message) 
	{
	    return outlet(outletIdx, message, Atom.emptyArray);
    }

    /**
     * Sends a <code>char</code> as a message with no arguments.
     * The char is output as a symbol as opposed to its integer value.  
     * @param outletIdx outlet number to send from
     * @param c <code>char</char> to send
     *
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, char c)  {
    	return outlet(outletIdx, String.valueOf(c), Atom.emptyArray);
    }

    /**
     * Sends a 0 or a 1.  
     * @param outletIdx outlet number to send from
     * @param value <code>boolean</code> value to send - true sends a 1, false sends a 0
     *
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, boolean value)  {
   		return outlet(outletIdx, Atom.newAtom(value));
    }

    /**
     * Sends a message.
     * @param outletIdx outlet number to send from
     * @param message message to send
     * @param args <code>Atom</code> array to append as arguments to the message.
     * @return true if successful.
     * @throws NullPointerException if <tt>message</tt> or <tt>args</tt>
     * is <tt>null</tt> or if <tt>args</tt> contains <tt>null</tt>
     */
    public final boolean outlet(int outletIdx, String message, Atom args[]) {
		if (message == null) {
		    throw new NullPointerException("message cannot be null");
		}
		if (args == null) {
		    throw new NullPointerException("args argument cannot be null");
		}
		if (ArrayUtilities.containsNull(args)) {
		    throw new NullPointerException("args array cannot contain null");
		}
	    return doOutlet(outletIdx, message, args);
    }

    /**
     * Sends the contents of the Atom out.
     * Calls <code>outlet(int, int)</code>,
     * <code>outlet(int, float)</code>,
     * or <code>outlet(int, String)</code> depending on the content of the <code>Atom</code>. 
     * @param outletIdx outlet number to send from
     * @param value the <code>Atom</code> to send
     * @return true if successful.
     * @throws NullPointerException if <tt>a</tt>
     * equals or contains <tt>null</tt>
     */
	public final boolean outlet(int outletIdx, Atom value) {
		if (value.isInt())
			return outlet(outletIdx, value.getInt());
		else if (value.isFloat())
			return outlet(outletIdx, value.getFloat());
		else 
			return outlet(outletIdx, value.getString()); 
	}


    /**
     * Sends an array of <code>Atom</code>s.
     * If the output array begins with an <code>Atom</code>
     * that represents an <code>int</code> or a <code>float</code>,
     * the array is output as a list.
     * If the output array begins with an <code>Atom</code> 
     * that represents a <code>String</code>,
     * the array is output as a message and arguments.
     * If the output array contains only one item, the 
     * single item is output by calling
     * <code>outlet(int, Atom)</code>.
     * @param outletIdx outlet number to send from
     * @param array the array of <code>Atom</code>s to send
     * @return true if successful.
     * @throws NullPointerException if <tt>value</tt>
     * equals or contains <tt>null</tt>
     */
    public final boolean outlet(int outletIdx, Atom array[]) 
 	 {
		if ((array == null)||(array.length == 0)) {
		    //throw new NullPointerException("value cannot be null");
			return false;
		}
		if (ArrayUtilities.containsNull(array)) {
		    throw new NullPointerException("value array cannot contain null");
		}
		if (array.length==1) { //single-atom output
			return outlet(outletIdx, array[0]);
		} else if (array[0].isString()) { //message + arg output
			String message = array[0].getString();
			return doOutlet(outletIdx, message, Atom.removeFirst(array));
		} else {	//list output	
	    	return doOutlet(outletIdx, array);
	    }
    }


    /**
     * Sends a list of <code>int</code>s.
     * @param outletIdx outlet number to send from
     * @param value list of <code>int</code>s to send
     * @return true if successful.
     * @throws NullPointerException if <tt>value</tt>
     * equals or contains <tt>null</tt>
     */
    public final boolean outlet(int outletIdx, int value[]) {
		return doOutlet(outletIdx, value);
    }
    
    /**
     * Sends a list of <code>byte</code>s.
     * @param outletIdx outlet number to send from
     * @param value list of <code>byte</code>s to send
     * @return true if successful.
     * @throws NullPointerException if <tt>value</tt>
     * equals or contains <tt>null</tt>
     */
    public final boolean outlet(int outletIdx, byte value[]) {
		return outlet(outletIdx, Atom.newAtom(value));
    }
    
    /**
     * Sends a list of <code>short</code>s.
     * @param outletIdx outlet number to send from
     * @param value list of <code>short</code>s to send
     * @return true if successful.
     * @throws NullPointerException if <tt>value</tt>
     * equals or contains <tt>null</tt>
     */
    public final boolean outlet(int outletIdx, short value[]) {
		return outlet(outletIdx, Atom.newAtom(value));
    }
    
    /**
     * Sends a list of <code>long</code>s.
     * Warning, information can be lost in the conversion from <tt>long</tt>
     * to <tt>int</tt>!
     * @param outletIdx outlet number to send from
     * @param value list of <code>long</code>s to send
     * @return true if successful.
     * @throws NullPointerException if <tt>value</tt>
     * equals or contains <tt>null</tt>
     */
     public final boolean outlet(int outletIdx, long value[]) {
		return outlet(outletIdx, Atom.newAtom(value));
    }
    
     /**
      * Sends a list of <code>float</code>s.
      * @param outletIdx outlet number to send from
      * @param value list of <code>float</code>s to send
      * @return true if successful.
      * @throws NullPointerException if <tt>value</tt>
      * equals or contains <tt>null</tt>
      */
    public final boolean outlet(int outletIdx, float value[]) {
		return doOutlet(outletIdx, value);
    }
    
    /**
     * Sends a list of <code>double</code>s.
     * Warning, information can be lost in the conversion from java <tt>double</tt>
     * to native <tt>float</tt>!
     * @param outletIdx outlet number to send from
     * @param value list of <code>double</code>s to send
     * @return true if successful.
     * @throws NullPointerException if <tt>value</tt>
     * equals or contains <tt>null</tt>
     */
    public final boolean outlet(int outletIdx, double value[]) {
		return outlet(outletIdx, Atom.newAtom(value));
    }
    
    /**
     * Sends an array of <code>String</code>s.
     * The array will be sent as message (the first Atom) and arguments.
     * @param outletIdx outlet number to send from
     * @param value list of <code>String</code>s to send
     * @return true if successful.
     * @throws NullPointerException if <tt>value</tt>
     * equals or contains <tt>null</tt>
     */
    public final boolean outlet(int outletIdx, String value[]) {
		return outlet(outletIdx, Atom.newAtom(value));
    }

    /**
     * Sends an array of <code>char</code>s.
     * The array will be sent as message (the first Atom) and arguments.
     * @param outletIdx outlet number to send from
     * @param value list of <code>char</code>s to send
     * @return true if successful.
     * @throws NullPointerException if <tt>value</tt>
     * equals or contains <tt>null</tt>
     */
   public final boolean outlet(int outletIdx, char value[]) {
		return outlet(outletIdx, Atom.newAtom(value));
    }
    
   /**
    * Sends an array of <code>boolean</code>s.
    * The array will be sent as a list of 0s and 1s.
    * @param outletIdx outlet number to send from
    * @param array list of <code>boolean</code>s to send
    * @return true if successful.
    * @throws NullPointerException if <tt>value</tt>
    * equals or contains <tt>null</tt>
    */
    public final boolean outlet(int outletIdx, boolean array[])  {
    	return outlet(outletIdx, Atom.newAtom(array));
    }

//Begin msg convenience functions
 
     /**
     * Sends a message with an <code>int</code> arg.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value <code>int</code> argument to send
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, String msg, int value) 
	{
	    return doOutlet(outletIdx, msg, new Atom[]{Atom.newAtom(value)});
    }
     /**
     * Sends a message with a <code>byte</code> arg.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value <code>byte</code> argument to send
     * @return true if successful.
     */

    public final boolean outlet(int outletIdx,String msg, byte value) {
    	return doOutlet(outletIdx, msg, new Atom[]{Atom.newAtom((int)value)});
    }

     /**
     * Sends a message with an <code>short</code> arg.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value <code>short</code> argument to send
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx,String msg, short value) {
		return doOutlet(outletIdx, msg, new Atom[]{Atom.newAtom((int)value)});
    }

     /**
     * Sends a message with an <code>long</code> arg.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value <code>long</code> argument to send(will be truncated to int by Max)
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, String msg,long value) {
		return doOutlet(outletIdx, msg, new Atom[]{Atom.newAtom((int)value)});
    }


     /**
     * Sends a message with a <code>float</code> arg.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value <code>float</code> argument to send
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx,String msg, float value) 
	{
	    return doOutlet(outletIdx, msg,new Atom[]{Atom.newAtom(value)});
    }

    /**
     * Sends a message with a <code>double</code> arg.
     * Warning, since the Max float atom type is a 32 bit floating point number
     * information can be lost in the conversion from a java <code>double</code>
     * to a native <code>float</code>!
     * @param outletIdx outlet number to send from 
	* @param msg   Message to send
     * @param value <code>double</code> to send
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, String msg, double value)
    {
    		return doOutlet(outletIdx, msg, new Atom[]{Atom.newAtom((float)value)});
    }
    
     /**
     * Sends a message with a <code>String</code> arg.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value <code>String</code> argument to send
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, String msg, String value) 
	{
		return doOutlet(outletIdx, msg, new Atom[]{Atom.newAtom(value)});
    }

     /**
     * Sends a message with a <code>char</code> arg.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value <code>char</code> argument to send
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx,String msg, char c)  {
    	return doOutlet(outletIdx, msg, new Atom[]{Atom.newAtom(String.valueOf(c))});
    }
	
	/**
     * Sends a message with a <code>boolean</code> arg.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value <code>boolean</code> value to send - true sends a 1, false sends a 0
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, String msg, boolean value)  {
   		return doOutlet(outletIdx, msg, new Atom[]{Atom.newAtom(value)});
    }

   
	/**
     * Sends a message with an <code>Atom</code> arg.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value <code>Atom</code> value to send.
     * @return true if successful.
     */
	public final boolean outlet(int outletIdx, String msg, Atom value) {
			return doOutlet(outletIdx, msg, new Atom[]{value}); 
	}


	/**
     * Sends a message followed by an array of <code>int</code>s.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value array of<code>int</code> values to send.
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, String msg, int value[]) {
		return doOutlet(outletIdx, msg, value);
    }
    
  
	/**
     * Sends a message followed by an array of <code>byte</code>s.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value array of<code>byte</code> values to send.
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx,String msg, byte value[]) {
		return doOutlet(outletIdx,msg, Atom.newAtom(value));
    }
    

	/**
     * Sends a message followed by an array of <code>short</code>s.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value array of<code>short</code> values to send.
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, String msg, short value[]) {
		return doOutlet(outletIdx, msg, Atom.newAtom(value));
    }
    

	/**
     * Sends a message followed by an array of <code>long</code>s.
	 * !!WARNING: longs will be truncated to ints by Max. This is a convenience method.
	 * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value array of<code>long</code> values to send.
     * @return true if successful.
     */
     public final boolean outlet(int outletIdx, String msg, long value[]) {
		return doOutlet(outletIdx, msg, Atom.newAtom(value));
    }
    

	/**
     * Sends a message followed by an array of <code>float</code>s.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value array of<code>float</code> values to send.
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx,String msg, float value[]) {
		return doOutlet(outletIdx,msg, value);
    }
	
   	/**
     * Sends a message followed by an array of <code>double</code>s.
	 * !!WARNING: longs will be truncated to floats by Max. This is a convenience method.
	 * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value array of<code>float</code> values to send.
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx,String msg, double value[]) {
		return doOutlet(outletIdx, msg, Atom.newAtom(value));
    }
    
	/**
     * Sends a message followed by an array of <code>String</code>s.
	 * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value array of<code>String</code> values to send.
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx,String msg, String value[]) {
		return doOutlet(outletIdx, msg, Atom.newAtom(value));
    }

	/**
     * Sends a message followed by an array of <code>char</code>s.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value array of<code>char</code> values to send.
     * @return true if successful.
     */
   public final boolean outlet(int outletIdx, String msg, char value[]) {
		return doOutlet(outletIdx,msg, Atom.newAtom(value));
    }
	/**
     * Sends a message followed by an array of <code>boolean</code>s.
     * @param outletIdx outlet number to send from 
     * @param msg   Message to send
     * @param value array of<code>boolean</code> values to send.true will be sent as 1 and false will be sent as 0.
     * @return true if successful.
     */
    public final boolean outlet(int outletIdx, String msg, boolean array[])  {
    	return doOutlet(outletIdx,msg, Atom.newAtom(array));
    }
	


    // Native doOutlet methods.
     /**
     * Outlet number outletIdx should send a bang.
     * <br>
     * Internal method.
     * @return true if output occurred, false if not.
     */
    private native boolean doOutletBang(int outletIdx);

    /**
     * Outlet number outletIdx should send an int value.
     * <br>
     * Internal method.
     * @return true if output occurred, false if not.
     */
    private final native boolean doOutlet(int outletIdx, int value);

    /**
     * Outlet number outletIdx should send a float value.
     * <br>
     * Internal method.
     * @return true if output occurred, false if not.<br>
     */
    private final native boolean doOutlet(int outletIdx, float value);

    /**
     * Outlet number outletIdx should send a message.
     * <br>
     * Internal method.
     * @return true if output occurred, false if not.
     */
    private final native boolean doOutlet(int outletIdx, String message, Atom args[]);

    /**
     * Outlet number outletIdx should send a list value.
     * <br>
     * Internal method.
     * @return true if output occurred, false if not.
     */
    private final native boolean doOutlet(int outletIdx, Atom value[]);
	
	/**
     * Outlet number outletIdx should send a list value.
     * <br>
     * Internal method.
     * @return true if output occurred, false if not.
     */
    private final native boolean doOutlet(int outletIdx, int value[]);
	/**
     * Outlet number outletIdx should send a list value.
     * <br>
     * Internal method.
     * @return true if output occurred, false if not.
     */
    private final native boolean doOutlet(int outletIdx, float value[]);
	
	/**
     * Outlet number outletIdx should send a list value.
     * <br>
     * Internal method.
     * @return true if output occurred, false if not.
     */
    private final native boolean doOutlet(int outletIdx, String msg, int value[]);
	
	
	/**
     * Outlet number outletIdx should send a list value.
     * <br>
     * Internal method.
     * @return true if output occurred, false if not.
     */
    private final native boolean doOutlet(int outletIdx,String msg, float value[]);
	

	//I'm so High!!!
    /**
     * Sends a bang out an outlet in the timer thread if overdrive is on.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     */
    public final boolean outletBangHigh(int outletIdx) 
	{
		return doOutletBangHigh(outletIdx);
    }

    /**
     * Sends an <tt>int</tt> out an outlet in the timer thread if overdrive is on.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param value the value to send
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, int value) 
	{
	    return doOutletHigh(outletIdx, value);
    }

    /**
     * Sends a <tt>float</tt> out an outlet in the timer thread.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param value the value to send
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, float value) 
	 {
	    return doOutletHigh(outletIdx, value);
     }

    /**
     * Sends a <tt>double</tt> out an outlet in the timer thread.
     * This is the same as using a "delay 0" object in the Max world.
     * Warning, loss of precision can occur when converting from java <tt>double</tt>
     * to native <tt>float</tt>!
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param value the value to send
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, double value) 
	{
	    return doOutletHigh(outletIdx,(float)value);	
    }
    
    /**
     * Sends a message with no arguments out an outlet in the timer thread.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param message the message to send
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, String message) 
	{
    	return outletHigh(outletIdx, message, Atom.emptyArray);
    }

    /**
     * Sends a message and arguments out an outlet in the timer thread.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param message the message to send
     * @param args the arguments to append to the message
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, String message, Atom args[]) 
	 {

	if (message == null) {
	    throw new NullPointerException("message cannot be null");
	}
	if (args == null) {
	    throw new NullPointerException("args argument cannot be null");
	}
	if (ArrayUtilities.containsNull(args)) {
	    throw new NullPointerException("args array cannot contain null");
	}
	    return doOutletHigh(outletIdx, message, args);

    }
	
	    /**
     * Sends a message and arguments out an outlet in the timer thread.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param message the message to send
     * @param args the arguments to append to the message
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, String message, int values[]) 
	 {

		return doOutletHigh(outletIdx,message,values);

    }
		    /**
     * Sends a message and arguments out an outlet in the timer thread.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param message the message to send
     * @param args the arguments to append to the message
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, String message, float values[]) 
	 {

		return doOutletHigh(outletIdx,message,values);

    }

    /**
     * Sends a list out an outlet in the timer thread.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param value the list of <tt>Atom</tt>s to send
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, Atom value[]) 
	{

	if (value == null) {
	    throw new NullPointerException("value cannot be null");
	}
	if (ArrayUtilities.containsNull(value)) {
	    throw new NullPointerException("value array cannot contain null");
	}
	    return doOutletHigh(outletIdx, value);
	
    }
	
	    /**
     * Sends a list out an outlet in the timer thread.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param value the list of <tt>Atom</tt>s to send
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, int values[]) 
	{
	    return doOutletHigh(outletIdx, values);
    }
	
	/**
     * Sends a list out an outlet in the timer thread.
     * This is the same as using a "delay 0" object in the Max world.
     * @return true if successful.
     * @param outletIdx the index of the outlet to send through
     * @param value the list of <tt>Atom</tt>s to send
     * @throws IndexOutOfBoundsException if <tt>outletIdx</tt> is out of range
     */
    public final boolean outletHigh(int outletIdx, float values[]) 
	{
	    return doOutletHigh(outletIdx, values);
    }


    // Native doOutletHigh methods.
   
    private native boolean doOutletBangHigh(int outletIdx);
    private final native boolean doOutletHigh(int outletIdx, int value);
    private final native boolean doOutletHigh(int outletIdx, float value);
    private final native boolean doOutletHigh(int outletIdx, String message, Atom args[]);
    private final native boolean doOutletHigh(int outletIdx, Atom value[]);
	private final native boolean doOutletHigh(int outletIdx, int value[]);
    private final native boolean doOutletHigh(int outletIdx, float value[]);
    private final native boolean doOutletHigh(int outletIdx, String msg, int value[]);
    private final native boolean doOutletHigh(int outletIdx,String msg, float value[]);
	

    //
    // Other utility methods.
    //

    /**
     * Return our peer.
     */
    private long getPeer() {
	return mPeer;
    }



    /**
     * Return a human-readable version of this object.
     */
    public String toString() {
	StringBuffer sb = new StringBuffer();
	sb.append(getClass().getName());
	sb.append("[");
	if (mName != null) {
	    sb.append("name: '");
	    sb.append(mName);
	    sb.append("'; ");
	}
	sb.append(mInlets.length);
	sb.append(" in; ");
	sb.append(mOutlets.length);
	sb.append(" out]");

	return sb.toString();
    }

/**
 * Called by Max when the user saves the patch.
 * <br>
 * The typical use of this method is to save some information 
 * about the current state of the object in the patcher file.  
 * This is accomplished using the <tt>embedMessage</tt>
 * method. <tt>embedMessage</tt> is only valid if called from
 * within the save method and in no other context. This a typical
 * overridden save method will consist of one or more embdedMessage
 * calls.The <tt>save</tt> method does nothing by default - override this method in 
 * your <code>MaxObject</code> subclass to make it do 
 * something useful.
 * 
 * @see #embedMessage
 */
protected void save()
{
	
}

/**
 * Called by Max at loadbang time (ie, when the enclosing patch has finished loading).
 * Override this method in your <code>MaxObject</code> subclass 
 * to make it do something useful.
 * Your class can respond to the loadbang message in any way
 * that it wants, but before implementing a method that responds 
 * to loadbang, keep in mind that the user can connect your 
 * object to the outlet of a loadbang object to perform initialization if 
 * necessary.  Note that you do not get the loadbang message
 * when the user creates a new instance of your object in the 
 * Patcher window, only when a Max file containing your class is loaded from disk.
 * You can assume that all elements of a patch have been loaded and all connections
 * have been made and are valid when the loadbang message is sent to your object.
 */
protected void loadbang()
{

}

/**
 * Called by Max when the user double-clicks on the object box. 
 * Override this method in your <code>MaxObject</code> subclass 
 * to make it do something useful,for example, opening up a window to display
 * information about the state of your object.
 */
protected void dblclick()
{

}

/**
 * Embed messages in a patcher file for initialization purposes.
 * <br>
 * Use this method to save messages that will be sent to the object
 * when the patch is next opened.  For instance, the following class 
 * saves the <tt>int</tt> variable <tt>importantData</tt> in the 
 * patcher file.  When the patcher file is opened, the <tt>set</tt>
 * method is called with the old value of <tt>importantData</tt>, just 
 * as if it had been sent in one of the object's inlets.
 * 
 * <br>
 * <pre>
public class saveExample extends MaxObject {
	
	private int importantData;
	
	public void set(int i) {
		importantData = i;
	}
	
	public void get() {
		outlet(0, importantData);
	}
	
	public void save() {
		Atom[] data = new Atom[] {Atom.newAtom(importantData)};
		embedMessage("set", data);
	}
	
}
</pre>
 * 
 * <tt>embedmessage</tt> can only be called from within the <tt>save</tt>
 * method and is valid in no other context.
 * <br>
 * 
 * @param msg the name of the method that will be called
 * @param args the arguments to pass to the method
 * @see com.cycling74.max.MaxObject#save()
 */
protected void embedMessage(String msg, Atom[] args) {
	if (msg.length() > 0)
		doEmbedMessage(msg, args);
}

private native void doEmbedMessage(String msg, Atom[] args) ;


//bbn- _get_all_methods and _get_all_fields put together
//arrays of all methods and fields in all classes below MaxObject

// tml- I changed this so that it also includes max object. that way we can
// have messages inherited by default. For example viewsource.
//Any method you don't want to be exposed as a method declare as protected.
private Method[] _get_all_methods(Object o) {
	Class c = o.getClass();
	Method[] methods = c.getDeclaredMethods();
	c = c.getSuperclass();
	while (c.getName() != "java.lang.Object") {
		Method[] tempa = c.getDeclaredMethods();
		Method[] tempb = new Method[methods.length+tempa.length];
		System.arraycopy(methods, 0, tempb, 0, methods.length);
		System.arraycopy(tempa, 0, tempb, methods.length, tempa.length);
		methods = tempb;
		c = c.getSuperclass();
	}
	return methods;
}
//we only want to return constructors valis for creating an instance
//of this class
private Constructor[] _get_all_constructors(Object o) {
	Class c = o.getClass();
	Constructor[] constructors = c.getDeclaredConstructors();
	//c = c.getSuperclass();
	//while (c.getName() != "java.lang.Object") {
	//	Constructor[] tempa = c.getDeclaredConstructors();
	//	Constructor[] tempb = new Constructor[constructors.length+tempa.length];
	//	System.arraycopy(constructors, 0, tempb, 0, constructors.length);
	//	System.arraycopy(tempa, 0, tempb, constructors.length, tempa.length);
	//	constructors = tempb;
	//	c = c.getSuperclass();
	//}
	return constructors;
}


private Method[] getAllMethodsButMaxObject(Object o) {
	Class c = o.getClass();
	Method[] methods = c.getDeclaredMethods();
	c = c.getSuperclass();
	while (c.getName() != "com.cycling74.max.MaxObject") {
		Method[] tempa = c.getDeclaredMethods();
		Method[] tempb = new Method[methods.length+tempa.length];
		System.arraycopy(methods, 0, tempb, 0, methods.length);
		System.arraycopy(tempa, 0, tempb, methods.length, tempa.length);
		methods = tempb;
		c = c.getSuperclass();
	}
	return methods;
}

private Field[] _get_all_fields(Object o) {
	Class c = this.getClass();
	Field[] fields = c.getDeclaredFields();
	c = c.getSuperclass();
	while (c.getName() != "com.cycling74.max.MaxObject") {
		Field[] tempa = c.getDeclaredFields();
		Field[] tempb = new Field[fields.length+tempa.length];
		System.arraycopy(fields, 0, tempb, 0, fields.length);
		System.arraycopy(tempa, 0, tempb, fields.length, tempa.length);
		fields = tempb;
		c = c.getSuperclass();
	}
	return fields;
}


/**
 * Declare an attribute with default getter and setter methods.
 * Deafult getter and setters can only be generated for member variables
 * with primative type int,float,boolean etc and String and Atom.
 * Primative array types are also supported as well as String[] and Atom[]
 * @param name the name of the variable to declare as an attribute
 */
protected void declareAttribute(String name)
{
	declareAttribute(name, null, null,true,true);
}

/**
 * Declare an attribute with default getter method.
 * Deafult getters can only be generated for member variables
 * with primative type int,float,boolean etc and String and Atom.
 * Primative array types are also supported as well as String[] and Atom[]
 * @param name the name of the variable to declare as an attribute
 */
protected void declareReadOnlyAttribute(String name)
{
	declareAttribute(name, null, null,true,false);
}

/**
 * Declare an attribute with specific getter and setter methods.
 * <br>
 * A setter method must return <tt>void</tt> and take as an argument
 * the same type as the attribute variable.
 * A getter method must take no arguments and return an 
 * <code>Atom</code> array.  Tthis array will be appended as arguments
 * to the name of the attribute and sent
 * out of the info outlet.
 * <br>
 * To only set either the getter or the setter, declare the other
 * method as null.  For instance,
 * <pre>
 * declareAttribute("myAttribute", null, "setMyAttribute");
 * </pre>
 * uses a specific setter method and the default getter method.
 * <br>
 * 
 * If an object is instantiated with attribute arguments (eg, @myAttribute)
 * the setter method will be executed before the parent Max patch is well-formed 
 * and stable.  Therefore it is <b>critical</b> that the setter method 
 * not execute any method that interacts with the Max universe - for instance, 
 * sending data out an outlet.  Setting a <code>MaxClock</code> or 
 * <code>MaxQelem</code> whose <code>Executable</code> will interact with the
 * Max universe has the potential to be similarly disastrous. For instance,
 * an outlet call could be executed before the outlet is connected to anything
 * valid. 
 * @param name the name of the variable to declare as an attribute
 * @param getter the name of the getter method
 * @param setter the name of the setter method
 */
protected void declareAttribute(String name,String getter,String setter)
 {
 	declareAttribute(name,getter,setter,true,true);
 } 
 /**
 * Declare a readonly attribute with a specific getter method.
 * <br>.
 * A getter method must take no arguments and return an 
 * <code>Atom</code> array.  This array will be appended as arguments
 * to the name of the attribute and sent
 * out of the info outlet.
 *
 * @param name the name of the variable to declare as a readonly attribute
 * @param getter the name of the getter method
 */
protected void declareReadOnlyAttribute(String name,String getter)
{
	declareAttribute(name,getter,null,true,false);
}

private void declareAttribute(String name,String getter,String setter,boolean gettable,boolean settable) 
{
	AttributeInfo a = new AttributeInfo();
	a.name      = name;
	a.settable  = false;
	a.gettable  = false;
	a.isvirtual = 1;
	a.j_f_type  =  "virtual";
	a.m_f_type  =  "virtual";
	
	Field f = null;	
	//check if we are binding to a field or creating a virtual attribute
	Field[] fields = _get_all_fields(this);
	
	for(int i = 0; i < fields.length; i++)
	{		
		f = fields[i];
		if(f.getName().equals(a.name))
		{  
			a.j_f_type  = "object";
			a.isvirtual = 2; //object field
			if(_is_valid_primative_attr(f))
			{
				Method getterM = null;
				Method setterM = null;
				
				Object[] nfo = _get_attr_info(f);
				a.m_f_type  = (String)nfo[0];
				a.j_f_type  = (String)nfo[1];
				a.isvirtual = 0;
				a.settable  = settable;
				a.gettable  = gettable;
				
				if(setter != null)
				{
				 	if((setterM = _is_valid_attr_setter(setter)) != null)//getter method exists and is a proper getter
					{
						Object[] sigs;
						a.setter   = setter;
						sigs = _build_sigs(setterM);						
						a.setter_sig    = (String)sigs[1];
						a.setter_jptypes = (String)sigs[3];
					}
					else
						;//System.out.println("warning: ignoring attribute "+a.name+" setter. It is a not valid. Will use default setter.");
				}								
				if(getter != null) 
				{
				 	if((getterM = _is_valid_attr_getter(getter)) != null)//setter method exists and is a proper setter
					{
						Class rt = getterM.getReturnType();
						a.getter   = getter;						
						if(rt.equals(Integer.TYPE))
							a.getter_sig = ("()I");
						else if	(rt.equals(Long.TYPE))
							a.getter_sig = ("()J");
						else if	(rt.equals(Short.TYPE))
							a.getter_sig = ("()S");
						else if	(rt.equals(Byte.TYPE))
							a.getter_sig = ("()B");
						else if	(rt.equals(Float.TYPE))
							a.getter_sig = ("()F");
						else if	(rt.equals(Double.TYPE))
							a.getter_sig = ("()D");
						else if	(rt.equals(Boolean.TYPE))
							a.getter_sig = ("()Z");
						else if	(rt.equals(Character.TYPE))
							a.getter_sig = ("()C");
						else if(rt.equals(STRING_CLASS))
							a.getter_sig = ("()Ljava/lang/String;");
						else if(rt.equals(ATOM_ARRAY_CLASS))
							a.getter_sig = ("()[Lcom/cycling74/max/Atom;");
						else if(rt.equals(BOOL_ARRAY_CLASS))
							a.getter_sig = ("()[Z");
						else if(rt.equals(BYTE_ARRAY_CLASS))
							a.getter_sig = ("()[B");
						else if(rt.equals(CHAR_ARRAY_CLASS))
							a.getter_sig = ("()[C");
						else if(rt.equals(SHORT_ARRAY_CLASS))
							a.getter_sig = ("()[S");
						else if(rt.equals(INT_ARRAY_CLASS))
							a.getter_sig = ("()[I");
						else if(rt.equals(LONG_ARRAY_CLASS))
							a.getter_sig = ("()[J");
						else if(rt.equals(FLOAT_ARRAY_CLASS))
							a.getter_sig = ("()[F");
						else if(rt.equals(DOUBLE_ARRAY_CLASS))
							a.getter_sig = ("()[D");
						else if(rt.equals(STRING_ARRAY_CLASS))
							a.getter_sig = ("()[Ljava/lang/String;");
					}
					else
						;//System.out.println("warning: ignoring attribute "+a.name+" getter. It is a not valid. Will use default getter.");
				}
				_make_java_callable_attribute(a.name,a,getterM,setterM,f);		
				_attributes.addElement(a);
				return;
			}//end if valid primative attr
		}
	}
	//Not a primative field. Must be an object field or totally virtual attribute
	Method getterM = null;
	Method setterM = null;
	if(setter == null)
		a.settable =  false;
	else if((setterM = _is_valid_attr_setter(setter))  != null)//setter method exists and is a proper setter
	{
		Object[] sigs;
		a.settable = true;
		a.setter = setter;
		sigs = _build_sigs(setterM);
		a.setter_sig    = (String)sigs[1];
		a.setter_jptypes = (String)sigs[3];
	}
	else
		System.out.println("warning: ignoring virtual attribute "+a.name+" setter. It is a not valid.");

	if(getter == null)
		a.gettable =  false;
	else if((getterM = _is_valid_attr_getter(getter)) != null)//getter method exists and is a proper getter
	{
		a.getter   = getter;
		a.gettable  = gettable;						
	
		Class rt = getterM.getReturnType();
	
		if(rt.equals(Integer.TYPE))
			a.getter_sig = ("()I");
		else if	(rt.equals(Long.TYPE))
			a.getter_sig = ("()J");
		else if	(rt.equals(Short.TYPE))
			a.getter_sig = ("()S");
		else if	(rt.equals(Byte.TYPE))
			a.getter_sig = ("()B");
		else if	(rt.equals(Float.TYPE))
			a.getter_sig = ("()F");
		else if	(rt.equals(Double.TYPE))
			a.getter_sig = ("()D");
		else if	(rt.equals(Boolean.TYPE))
			a.getter_sig = ("()Z");
		else if	(rt.equals(Character.TYPE))
			a.getter_sig = ("()C");
		else if(rt.equals(STRING_CLASS))
			a.getter_sig = ("()Ljava/lang/String;");
		else if(rt.equals(ATOM_ARRAY_CLASS))
			a.getter_sig = ("()[Lcom/cycling74/max/Atom;");
		else if(rt.equals(BOOL_ARRAY_CLASS))
			a.getter_sig = ("()[Z");
		else if(rt.equals(BYTE_ARRAY_CLASS))
			a.getter_sig = ("()[B");
		else if(rt.equals(CHAR_ARRAY_CLASS))
			a.getter_sig = ("()[C");
		else if(rt.equals(SHORT_ARRAY_CLASS))
			a.getter_sig = ("()[S");
		else if(rt.equals(INT_ARRAY_CLASS))
			a.getter_sig = ("()[I");
		else if(rt.equals(LONG_ARRAY_CLASS))
			a.getter_sig = ("()[J");
		else if(rt.equals(FLOAT_ARRAY_CLASS))
			a.getter_sig = ("()[F");
		else if(rt.equals(DOUBLE_ARRAY_CLASS))
			a.getter_sig = ("()[D");
		else if(rt.equals(STRING_ARRAY_CLASS))
			a.getter_sig = ("()[Ljava/lang/String;");
	}
	else
		System.out.println("warning: ignoring virtual attribute "+a.name+" getter. It is a not valid.");
	
	if(!a.gettable && !a.settable)
	{
		System.out.println("warning: ignoring object attribute "+a.name+" alltogether. No getter or setter defined.");
		return;
	}
	_make_java_callable_attribute(a.name,a,getterM,setterM,null);			
	_attributes.addElement(a);
}

private native void _register_message(String name,int num_params,String java_sig, String mxj_param_types, String java_param_types);//,Object meth);
private native void _register_attribute(AttributeInfo ai);

private Object[] s_meth_desc  = null;
private Object[] s_const_desc = null;

private void _init_mxj_message_table()
{

	int i;

	if(s_meth_desc == null || s_const_desc == null)
		_init_mxj_message_table_low();

	Object[] sigs;
	for(i = 0; i < s_meth_desc.length;i++)
	{	
		sigs = (Object[])s_meth_desc[i];
		//System.out.println("Registering method: "+(String)sigs[0]+" param num: "+(Integer)sigs[4]+" java sig: "+ (String)sigs[1]+" mxj_p_types: "+(String)sigs[2]+" java p_types: "+(String)sigs[3]);
		_register_message((String)sigs[0],((Integer)sigs[4]).intValue(),(String)sigs[1],(String)sigs[2], (String)sigs[3]);//,sigs[5]);		
	}	
	for(i = 0; i < s_const_desc.length;i++)
	{	
		sigs = (Object[])s_const_desc[i];
		//System.out.println("Registering Const: "+(String)sigs[0]+" param num: "+(Integer)sigs[4]+" java sig: "+ (String)sigs[1]+" mxj_p_types: "+(String)sigs[2]+" java p_types: "+(String)sigs[3]);
		_register_message((String)sigs[0],((Integer)sigs[4]).intValue(),(String)sigs[1],(String)sigs[2], (String)sigs[3]);//,sigs[5]);	
	}

}

private void _init_mxj_message_table_low()
{
	Method m;
	Method[] meths       = _get_all_methods(this);
	Constructor c;
	Constructor[] consts = _get_all_constructors(this);
	Object[] tmp;
	int i,cnt = 0;
	
	tmp = new Object[meths.length];
	for(i = 0; i < meths.length; i++)
	{	
		m = meths[i];
		if(_is_mxj_callable_method(m))
		{	
			tmp[cnt] = _build_sigs(m);
			cnt++;
		}
	}
	s_meth_desc = new Object[cnt];
	for(i = 0; i < cnt;i++)
	{
		s_meth_desc[i] = tmp[i];
	}
	//System.arraycopy(tmp,0,s_meth_desc,0,cnt);
	cnt = 0;
	//Now we are putting the constructors in there too...	
	tmp = new Object[consts.length];
	for(i = 0; i < consts.length; i++)
	{	
		c = consts[i];
		if(_is_mxj_callable_constructor(c))
		{
			tmp[cnt] = _build_sigs(c,true);
			cnt++;
		}
	}
	s_const_desc = new Object[cnt];
	for(i = 0; i < cnt;i++)
	{
		s_const_desc[i] = tmp[i];
	}
	
	//System.arraycopy(tmp,0,s_const_desc,0,cnt);
}


private void _init_mxj_attr_table()
{

	for(int i = 0; i < _attributes.size();i++)
	{
		AttributeInfo ai = (AttributeInfo)_attributes.elementAt(i);	
		//post(ai.toString());
		//maybe we can verify stuff here!!! i.e methods exist etc...
		_register_attribute(ai); 
	}

}

//default call is for Method objects
private Object[] _build_sigs(Object m)
{
	return _build_sigs(m,false);
}

private Object[] _build_sigs(Object m,boolean isconstructor)
{
	int i;
	Class[] params;
	String name;
	StringBuffer java_sig    = new StringBuffer("(");
	StringBuffer mxj_ptypes  = new StringBuffer();
	StringBuffer java_ptypes = new StringBuffer();

	if(!isconstructor)
	{
     	params = ((Method)m).getParameterTypes();
		name = ((Method)m).getName();
	}
	else
	{
	 	params = ((Constructor)m).getParameterTypes();
		name = "<init>";
	}
	for( i= 0; i < params.length;i++)
	{
		if(params[i].equals(Integer.TYPE))
		{	
			java_sig.append('I');
			mxj_ptypes.append('I');
			java_ptypes.append('I');
		}
		else if	(params[i].equals(Long.TYPE))
		{
			java_sig.append('J');
			mxj_ptypes.append('I');
			java_ptypes.append('J');
		}
		else if	(params[i].equals(Short.TYPE))
		{
			java_sig.append('S');
			mxj_ptypes.append('I');
			java_ptypes.append('S');
		}
		else if	(params[i].equals(Byte.TYPE))
		{
			java_sig.append('B');
			mxj_ptypes.append('I');
			java_ptypes.append('B');
		}
		else if	(params[i].equals(Float.TYPE))
		{
			java_sig.append('F');
			mxj_ptypes.append('F');
			java_ptypes.append('F');
		}
		else if	(params[i].equals(Double.TYPE))
		{
			java_sig.append('D');
			mxj_ptypes.append('F');
			java_ptypes.append('D');
		}
		else if	(params[i].equals(Boolean.TYPE))
		{
			java_sig.append('Z');
			mxj_ptypes.append('I');
			java_ptypes.append('Z');
		}
		else if	(params[i].equals(Character.TYPE))
		{
			java_sig.append('C');
			mxj_ptypes.append('I');
			java_ptypes.append('C');
		}
		else if(params[i].equals(STRING_CLASS))
		{
			java_sig.append("Ljava/lang/String;");
			mxj_ptypes.append('s');
			java_ptypes.append('s');//upper case 'S' is used for Short
		}
		else if(params[i].equals(ATOM_ARRAY_CLASS))
		{
		    java_sig.append("[Lcom/cycling74/max/Atom;");
			mxj_ptypes.append('G'); //GIMME
			java_ptypes.append('G');
		}
		else if(params[i].equals(BOOL_ARRAY_CLASS))
		{
		    java_sig.append("[Z");
			mxj_ptypes.append("[I"); //Array??
			java_ptypes.append("[Z");
		}
		else if(params[i].equals(BYTE_ARRAY_CLASS))
		{
		    java_sig.append("[B");
			mxj_ptypes.append("[I"); //Array??
			java_ptypes.append("[B");
		}
		else if(params[i].equals(CHAR_ARRAY_CLASS))
		{
		    java_sig.append("[C");
			mxj_ptypes.append("[I"); //Array??
			java_ptypes.append("[C");
		}
		else if(params[i].equals(SHORT_ARRAY_CLASS))
		{
		    java_sig.append("[S");
			mxj_ptypes.append("[I"); //Array??
			java_ptypes.append("[S");
		}
		else if(params[i].equals(INT_ARRAY_CLASS))
		{
		    java_sig.append("[I");
			mxj_ptypes.append("[I"); //Array??
			java_ptypes.append("[I");
		}
		else if(params[i].equals(LONG_ARRAY_CLASS))
		{
		    java_sig.append("[J");
			mxj_ptypes.append("[I"); //Array??
			java_ptypes.append("[J");
		}
		else if(params[i].equals(FLOAT_ARRAY_CLASS))
		{
		    java_sig.append("[F");
			mxj_ptypes.append("[F"); //Array??
			java_ptypes.append("[F");
		}
		else if(params[i].equals(DOUBLE_ARRAY_CLASS))
		{
		    java_sig.append("[D");
			mxj_ptypes.append("[F"); //Array??
			java_ptypes.append("[D");
		}
		else if(params[i].equals(STRING_ARRAY_CLASS))
		{
		    java_sig.append("[Ljava/lang/String;");
			mxj_ptypes.append("[s"); //Array??
			java_ptypes.append("[s");
		}
	}
	if(params.length == 0)
	{
		mxj_ptypes.append('V');	
		java_ptypes.append('V');
	}
	java_sig.append(")V");
	return (new Object[]{ name,java_sig.toString(),mxj_ptypes.toString(),java_ptypes.toString(),new Integer(i),m });
}

private Method _is_valid_attr_setter(String methodname)
{	
	Method[] methods = _get_all_methods(this);
	for(int i = 0; i < methods.length;i++)
	{
		Method m = methods[i];
		if(methods[i].getName().equals(methodname) &&
		_is_attr_callable_set_method(methods[i]))
			return m;
	}
	return null;
}

private Method _is_valid_attr_getter(String methodname)
{
	Method[] methods = _get_all_methods(this);
	for(int i = 0; i < methods.length;i++)
	{
		Method m = methods[i];
		if( m.getName().equals(methodname) &&
			m.getParameterTypes().length == 0 &&
			!Modifier.isStatic(m.getModifiers()))
			{
				Class rt = m.getReturnType();
				if(	rt.equals(Integer.TYPE) ||
					rt.equals(Float.TYPE)   ||
					rt.equals(STRING_CLASS) ||
					rt.equals(Short.TYPE)   ||
					rt.equals(Long.TYPE)    ||
					rt.equals(Character.TYPE)    ||
					rt.equals(Byte.TYPE)    ||
					rt.equals(Double.TYPE)    ||
					rt.equals(Long.TYPE)    ||
					rt.equals(Boolean.TYPE) ||
					rt.equals(ATOM_ARRAY_CLASS) ||
					rt.equals(BOOL_ARRAY_CLASS) ||
					rt.equals(BYTE_ARRAY_CLASS) ||
					rt.equals(CHAR_ARRAY_CLASS) ||
					rt.equals(SHORT_ARRAY_CLASS) ||
					rt.equals(INT_ARRAY_CLASS) ||
					rt.equals(LONG_ARRAY_CLASS) ||
					rt.equals(FLOAT_ARRAY_CLASS) ||	
					rt.equals(DOUBLE_ARRAY_CLASS) ||
					rt.equals(STRING_ARRAY_CLASS))				
						return m;
	
			}
	}
	return null;
}
//this is because attribute getter and setter can be attched to private methods.
//factor this at some point
private boolean _is_attr_callable_set_method(Method m)
{
	int modifiers   = m.getModifiers();
	
	if(!Modifier.isStatic(modifiers) &&
	   (m.getReturnType().equals(void.class))) 
	   {
			Class[] params = m.getParameterTypes();
			for(int i= 0; i < params.length;i++)
			{
				if(	params[i].equals(Integer.TYPE) ||
					params[i].equals(Float.TYPE)   ||
					params[i].equals(STRING_CLASS) ||
					params[i].equals(Short.TYPE)   ||
					params[i].equals(Long.TYPE)    ||
					params[i].equals(Character.TYPE)    ||
					params[i].equals(Byte.TYPE)    ||
					params[i].equals(Double.TYPE)    ||
					params[i].equals(Long.TYPE)    ||
					params[i].equals(Boolean.TYPE))
					continue;
				else if(i == 0 && params[i].equals(ATOM_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(BOOL_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(BYTE_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(CHAR_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(SHORT_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(INT_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(LONG_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(FLOAT_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(DOUBLE_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(STRING_ARRAY_CLASS))
					return true;																	
				else
				  return false;
			}
		return true;
		
		}

	return false;
}

private boolean _is_mxj_callable_method(Method m)
{
	int modifiers   = m.getModifiers();
	
	if( Modifier.isPublic(modifiers) &&
       !Modifier.isStatic(modifiers) &&
	   (m.getReturnType().equals(void.class))) 
	   {
			Class[] params = m.getParameterTypes();
			for(int i= 0; i < params.length;i++)
			{
				if(	params[i].equals(Integer.TYPE) ||
					params[i].equals(Float.TYPE)   ||
					params[i].equals(STRING_CLASS) ||
					params[i].equals(Short.TYPE)   ||
					params[i].equals(Long.TYPE)    ||
					params[i].equals(Character.TYPE)    ||
					params[i].equals(Byte.TYPE)    ||
					params[i].equals(Double.TYPE)    ||
					params[i].equals(Long.TYPE)    ||
					params[i].equals(Boolean.TYPE))
					continue;
				else if(i == 0 && params[i].equals(ATOM_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(BOOL_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(BYTE_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(CHAR_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(SHORT_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(INT_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(LONG_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(FLOAT_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(DOUBLE_ARRAY_CLASS))
					return true;
				else if(i == 0 && params[i].equals(STRING_ARRAY_CLASS))
					return true;																	
				else
				  return false;
			}
		return true;
		
		}

	return false;
}

private boolean _is_mxj_callable_constructor(Constructor m)
{
	int modifiers   = m.getModifiers();
	
	Class[] params = m.getParameterTypes();
	for(int i= 0; i < params.length;i++)
	{
		if(	params[i].equals(Integer.TYPE) ||
			params[i].equals(Float.TYPE)   ||
			params[i].equals(STRING_CLASS) ||
			params[i].equals(Short.TYPE)   ||
			params[i].equals(Long.TYPE)    ||
			params[i].equals(Character.TYPE)    ||
			params[i].equals(Byte.TYPE)    ||
			params[i].equals(Double.TYPE)    ||
			params[i].equals(Long.TYPE)    ||
			params[i].equals(Boolean.TYPE))
			continue;
		else if(i == 0 && params[i].equals(ATOM_ARRAY_CLASS))
			return true;
		else if(i == 0 && params[i].equals(BOOL_ARRAY_CLASS))
			return true;
		else if(i == 0 && params[i].equals(BYTE_ARRAY_CLASS))
			return true;
		else if(i == 0 && params[i].equals(CHAR_ARRAY_CLASS))
			return true;
		else if(i == 0 && params[i].equals(SHORT_ARRAY_CLASS))
			return true;
		else if(i == 0 && params[i].equals(INT_ARRAY_CLASS))
			return true;
		else if(i == 0 && params[i].equals(LONG_ARRAY_CLASS))
			return true;
		else if(i == 0 && params[i].equals(FLOAT_ARRAY_CLASS))
			return true;
		else if(i == 0 && params[i].equals(DOUBLE_ARRAY_CLASS))
			return true;
		else if(i == 0 && params[i].equals(STRING_ARRAY_CLASS))
			return true;																	
		else
		  return false;
	}
	return true;
				
}

//returns max type and java type of field
//we are using strings because we may want to support
//array types [I etc..
private Object[] _get_attr_info(Field f)
{
	Class type = f.getType();
	
	String m_t = null;
	String j_t = null;
	Object[] ret = new Object[2];
	
	if(type.equals(Integer.TYPE))
		{	
			m_t = ("I");
			j_t = ("I");
			
		}
		else if	(type.equals(Long.TYPE))
		{
			m_t = ("I");
			j_t = ("J");
		}
		else if	(type.equals(Short.TYPE))
		{
			m_t = ("I");
			j_t = ("S");
		}
		else if	(type.equals(Byte.TYPE))
		{
			m_t = ("I");
			j_t = ("B");
		}
		else if	(type.equals(Float.TYPE))
		{
			m_t = ("F");
			j_t = ("F");
		}
		else if	(type.equals(Double.TYPE))
		{
			m_t = ("F");
			j_t = ("D");
		}
		else if	(type.equals(Boolean.TYPE))
		{
			m_t = ("I");
			j_t = ("Z");
		}
		else if	(type.equals(Character.TYPE))
		{
			m_t = ("I");
			j_t = ("C");
		}
		else if(type.equals(STRING_CLASS))
		{
			m_t = ("s");
			j_t = ("s");//upper case 'S' is used for Short
		}
		else if(type.equals(INT_ARRAY_CLASS))
		{	
			m_t = ("[I");
			j_t = ("[I");
			
		}
		else if	(type.equals(LONG_ARRAY_CLASS))
		{
			m_t = ("[I");
			j_t = ("[J");
		}
		else if	(type.equals(SHORT_ARRAY_CLASS))
		{
			m_t = ("[I");
			j_t = ("[S");
		}
		else if	(type.equals(BYTE_ARRAY_CLASS))
		{
			m_t = ("[I");
			j_t = ("[B");
		}
		else if	(type.equals(FLOAT_ARRAY_CLASS))
		{
			m_t = ("[F");
			j_t = ("[F");
		}
		else if	(type.equals(DOUBLE_ARRAY_CLASS))
		{
			m_t = ("[F");
			j_t = ("[D");
		}
		else if	(type.equals(BOOL_ARRAY_CLASS))
		{
			m_t = ("[I");
			j_t = ("[Z");
		}
		else if	(type.equals(CHAR_ARRAY_CLASS))
		{
			m_t = ("[I");
			j_t = ("[C");
		}
		else if(type.equals(STRING_ARRAY_CLASS))
		{
			m_t = ("[s");
			j_t = ("[s");//upper case 'S' is used for Short
		}
		ret[0] = m_t;
		ret[1] = j_t;
		return ret;
}
//this is used to determine whether or not we can use built in get and set methods
private boolean _is_valid_primative_attr(Field f)
{
	int modifiers   = f.getModifiers();
	
	if(!Modifier.isStatic(modifiers) &&
       !Modifier.isFinal(modifiers))
	   {
			Class type = f.getType();
			if(	type.equals(Integer.TYPE) ||
				type.equals(Float.TYPE)   ||
				type.equals(STRING_CLASS) ||
				type.equals(Short.TYPE)   ||
				type.equals(Long.TYPE)    ||
				type.equals(Character.TYPE) ||
				type.equals(Byte.TYPE)      ||
				type.equals(Double.TYPE)    ||
				type.equals(Long.TYPE)      ||
				type.equals(Boolean.TYPE) ||
				type.equals(BOOL_ARRAY_CLASS) ||
				type.equals(BYTE_ARRAY_CLASS) ||
				type.equals(CHAR_ARRAY_CLASS) ||
				type.equals(SHORT_ARRAY_CLASS) ||
				type.equals(INT_ARRAY_CLASS) ||
				type.equals(LONG_ARRAY_CLASS) ||
				type.equals(FLOAT_ARRAY_CLASS) ||
				type.equals(DOUBLE_ARRAY_CLASS) ||
				type.equals(STRING_ARRAY_CLASS))
					return true;
				else
				  return false;
		}

	return false;
}

/////BEGIN JAVA ATTRIBUTE ACCESS
private HashMap _j_attribute_table = new HashMap(16);
class AttributeEntry
{
	AttributeInfo ai;
	Method getterM;
	Method setterM;
	Field f;
	
	AttributeEntry(AttributeInfo attinfo,Method getter,Method setter, Field field)
	{
		ai       = attinfo;
		f        = field;
		getterM  = getter;
		setterM  = setter;

	}
}

private void _make_java_callable_attribute(String name,AttributeInfo ai, Method getter, Method setter,Field f)
{
	//this is so accessor methods and attributes can be declare private
	if(getter != null)
		getter.setAccessible(true);
	if(setter != null)
		setter.setAccessible(true);
	if(f != null)
		f.setAccessible(true);
				
	_j_attribute_table.put(name,new AttributeEntry(ai,getter,setter,f));
}

    /**
     * Get the current value of a declared attribute as a java.lang.Object. It is the responsibility
	 * of the programmer to cast this value to the correct type. Primatively typed attributes are returned as instances of
	 * their wrapper class. For instance an int attribute will be returned as a java.lang.Integer. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value as a java.lang.Object.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
	*/
public Object getAttr(String name)
{
	AttributeEntry ae = (AttributeEntry)_j_attribute_table.get(name);

	if(ae == null)
	{//attribute does not exist
		//error("(mxj) attribute "+name+" does not exist as an attribute."); 
		throw new MaxRuntimeException("attribute "+name+" does not exist as an attribute.");
		//return null;
	}	
	if(ae.ai.gettable)
	{
		Object val = null;
		if(ae.getterM == null)
		{
			try{
				val = ae.f.get(this);
			}catch(IllegalAccessException e)
			{
				e.printStackTrace();
			}
			return val;
		}
		else//we have a getter defined
		{
			try{
				val = ae.getterM.invoke(this,null);
			}catch(Exception e)
			{
				e.printStackTrace();
			}
			return val;
		}
	}
	else
	{
		return null;
	}
}
    /**
     * Set the current value of a declared attribute. It is the responsibility
	 * of the programmer that the underlying type of val matches the type of the declared attribute.
	 * Primatively typed attributes are set using instances of
	 * their wrapper class. For instance when setting an int attribute the val argument should be an instance of java.lang.Integer.
	 * Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @param  name the name of the previously declared attribute. 
	 * @param  val the value you wish to set it to. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
	*/
public void setAttr(String name,Object val)
{
	AttributeEntry ae = (AttributeEntry)_j_attribute_table.get(name);

	if(ae == null)
	{//attribute does not exist
		//error("(mxj) setAttr failed for "+name+". It may not be declared as an attribute.");
		throw new MaxRuntimeException("attribute "+name+" does not exist as an attribute.");
		//return;
	}	
	if(ae.ai.settable)
	{
		if(ae.setterM == null)
		{
			try{
				ae.f.set(this,val);
			}catch(IllegalAccessException e)
			{
				e.printStackTrace();
			}
		}//we have a setter defined
		else
		{
			Object[] args = null;
			if(val.getClass().equals(OBJECT_ARRAY_CLASS))
				args = (Object[])val;
			else
				args = new Object[]{val};
			try{
				ae.setterM.invoke(this,args);
			}catch(Exception e)
			{
				e.printStackTrace();
			}
		}
	}
	else
	{
		error("(mxj) "+name+" is not a settable attribute.");
	}
}

public void setAttr(String name,Object[] args)
{
	setAttr(name,(Object)args);
}

    /**
     * Get the current value of a declared boolean attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value. 
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public boolean getAttrBool(String name)
{
	Object val = null;
	val = getAttr(name);

	return ((Boolean)val).booleanValue();
}

    /**
     * Get the current value of a declared boolean array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public boolean[] getAttrBoolArray(String name)
{
	return (boolean[])getAttr(name);
}
    /**
     * Get the current value of a declared byte attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public float getAttrByte(String name)
{
	Object val = null;
	val = getAttr(name);
	
	return ((Byte)val).byteValue();	
}
    /**
     * Get the current value of a declared byte array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public byte[] getAttrByteArray(String name)
{
	return (byte[])getAttr(name);
}

    /**
     * Get the current value of a declared char attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public char getAttrChar(String name)
{
	Object val = null;
	val = getAttr(name);
	
	return ((Character)val).charValue();

}

    /**
     * Get the current value of a declared char array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public char[] getAttrCharArray(String name)
{
	return (char[])getAttr(name);
}
    /**
     * Get the current value of a declared short attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public short getAttrShort(String name)
{
	Object val = null;
	val = getAttr(name);

	return ((Short)val).shortValue();

}
    /**
     * Get the current value of a declared short array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public short[] getAttrShortArray(String name)
{
	return (short[])getAttr(name);
}

    /**
     * Get the current value of a declared int attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public int getAttrInt(String name)
{
	Object val = null;
	val = getAttr(name);

	return ((Integer)val).intValue();

}
    /**
     * Get the current value of a declared int array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public int[] getAttrIntArray(String name)
{
	return (int[])getAttr(name);
}
    /**
     * Get the current value of a declared long attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public long getLongAttr(String name)
{
	Object val = null;
	val = getAttr(name);

	return ((Long)val).longValue();
	
}
	/**
     * Get the current value of a declared long array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public long[] getAttrLongArray(String name)
{
	return (long[])getAttr(name);
}
    /**
     * Get the current value of a declared float attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public float getAttrFloat(String name)
{
	Object val = null;
	val = getAttr(name);

	return ((Float)val).floatValue();
}
    /**
     * Get the current value of a declared float array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public float[] getAttrFloatArray(String name)
{
	return (float[])getAttr(name);
}
    /**
     * Get the current value of a declared double attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public double getAttrDouble(String name)
{
	Object val = null;
	val = getAttr(name);
	return ((Double)val).doubleValue();	
}
    /**
     * Get the current value of a declared double array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public double[] getAttrDoubleArray(String name)
{
	return (double[])getAttr(name);
}
    /**
     * Get the current value of a declared String attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public String getAttrString(String name)
{
	Object val = null;
	val = getAttr(name);
	return (String)val;
}
    /**
     * Get the current value of a declared String array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public String[] getAttrStringArray(String name)
{
	return (String[])getAttr(name);
}
    /**
     * Get the current value of a declared Atom array attribute. Attributes are declared using the
	 * <code>declareAttribute</code> methods of MaxObject.
     * @return attribute value.
     * @param  name the name of the previously declared attribute. 
	 * @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
     */
public Atom[] getAttrAtomArray(String name)
{
	return (Atom[])getAttr(name);
}

/**
* Set the current value of a declared boolean attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, boolean val)
{
	setAttr(name,new Boolean(val));
}
/**
* Set the current value of a declared boolean array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, boolean[] val)
{
	setAttr(name,(Object)val);
}
/**
* Set the current value of a declared byte attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, byte val)
{
	setAttr(name,new Byte(val));
}
/**
* Set the current value of a declared byte array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, byte[] val)
{
	setAttr(name,(Object)val);
}
/**
* Set the current value of a declared char attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, char val)
{
	setAttr(name,new Character(val));
}
/**
* Set the current value of a declared char array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, char[] val)
{
	setAttr(name,(Object)val);
}
/**
* Set the current value of a declared short attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, short val)
{
	setAttr(name,new Short(val));
}
/**
* Set the current value of a declared short array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, short[] val)
{
	setAttr(name,(Object)val);
}
/**
* Set the current value of a declared int attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, int val)
{
	setAttr(name,new Integer(val));
}
/**
* Set the current value of a declared int array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, int[] val)
{
	setAttr(name,(Object)val);
}
/**
* Set the current value of a declared long attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, long val)
{
	setAttr(name,new Long(val));
}
/**
* Set the current value of a declared long array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, long[] val)
{
	setAttr(name,(Object)val);
}
/**
* Set the current value of a declared float attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, float val)
{
	setAttr(name,new Float(val));
}
/**
* Set the current value of a declared float array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, float[] val)
{
	setAttr(name,(Object)val);
}
/**
* Set the current value of a declared double attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, double val)
{
	setAttr(name,new Double(val));
}
/**
* Set the current value of a declared double array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, double[] val)
{
	setAttr(name,(Object)val);
}
/**
* Set the current value of a declared String attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, String val)
{
	setAttr(name,new String(val));
}
/**
* Set the current value of a declared String array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, String[] val)
{
	setAttr(name,(Object)val);
}
/**
* Set the current value of a declared Atom array attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @param  name the name of the previously declared attribute.
* @param val the valuse you wish to set it to. 
* @throws MaxRuntimeException if attr named <tt>name</tt> does not exist.	
*/
public void setAttr(String name, Atom[] val)
{
	setAttr(name,(Object)val);
}

/**
* Get the array of AttributeInfo objects. One for each previously declared attribute. Attributes are declared using the
* <code>declareAttribute</code> methods of MaxObject.
* @return  array of com.cycling74.max.AttributeInfo objects.	
*/
public AttributeInfo[] getAttributeInfo()
{
	AttributeInfo[] ret = new AttributeInfo[_attributes.size()];
	for(int i = 0; i < _attributes.size();i++)
	{
		ret[i] = (AttributeInfo)_attributes.elementAt(i);	
	}
	return ret;
}
///END JAVA ATTRIBUTE ACCESS


   /**
    * If called during your constructor bail will cause no instance of
    * the class to be instantiated within max and the mxj box will show
    * up in the patcher as invalid. For instance if you required arguments
    * for your class which were not there you could use bail to print
    * a usage message to the console.
    <pre>
    public class myobject extends MaxObject
    {
        private int _arg = 0;
        //called when someone types [mxj myobject] into a box
        public myobject()
        {
            bail("usage: [mxj myobject required_arg]");
        }
        //called when someone types [mxj myobject 24] into a box
        public myobject(int required_arg)
        {
            _arg = required_arg;
        }
    
        ...blah blah blah    
    }
    </pre>
    * @param error or usage message you wish printed to the console.
    */
protected static void bail(String errormsg)
{
	error(errormsg);
	throw new MaxRuntimeException();
}

/*
public void classpath()
{
	String[] out = getClassPath();
	System.out.println("CURRENT CLASSPATH IS:");
	for(int i = 0; i < out.length;i++)
		System.out.println("    "+out[i]);
}
*/

private void _post_message_helpers()
    {
    	String method_suffix = "Helper";
    	Method[] meths = this.getClass().getMethods();
		post("--"+this.getClass().getName()+" responds to:");
		for(int i = 0; i < meths.length; i++)
		{
			
			if( Arrays.equals(meths[i].getParameterTypes(),GIMME_PARAM_TYPE) &&
				_is_mxj_callable_method(meths[i]))
				{
					Method m;
					String help_mess; 
					try {
					   m = this.getClass().getMethod(meths[i].getName() + method_suffix, null);
						//we should probably test the return type here - tml
					   help_mess = (String)m.invoke(this,null);
					   post("\t"+meths[i].getName()+":  "+help_mess);
	    			
	    			} catch (NoSuchMethodException nsme) {
		               post("\t"+meths[i].getName()+":");
	                } catch (IllegalAccessException iae) {
		              MaxObject.showException("Illegal access to "+meths[i].getName()+method_suffix, iae);
	                } catch (IllegalArgumentException iare) {
		              MaxObject.showException("Illegal argument", iare);
	                } catch (InvocationTargetException ite) {
		              MaxObject.showException(ite);
	                }
	            }
	          else //some other kind of method
	          	continue;  		
		
		}
		post("--");
		
    }
   
 /**
  * Invoke garbage collection via System.gc().
  */
 public void gc()
 {
 	System.gc();
 }
 
  /**
  * Force the next class to be loaded by mxj to be loaded with a new MXJClassLoader.
  */
 public void zap()
 {
	MXJClassLoader cl = MXJClassLoader.getInstance();
	if(cl != null)
		cl.zap();
 }
 


}
