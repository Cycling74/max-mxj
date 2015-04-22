package com.cycling74.max;

/**
 * <p>
 * The MaxQelem object can be used to defer execution of a method 
 * from the timer thread to the main thread.  This is critical when
 * the method to execute is heavyweight: drawing to the screen,
 * presenting the user with a dialog box, or performing a CPU-intensive
 * series of calculations that could adversely affect a patch's timing.
 * When the MaxQelem's set method is called the target function will be placed
 * on the low priority queue. Once a MaxQelem is set it remains set until the
 * target function is executed at which point it is "unset". Repeated calls
 * to "set" when the MaxQuelem is already set will have no effect since it
 * is already on the queue. This is useful in throttling operations that may
 * take a relatively long time to execute. For more information on qelems and
 * task scheduling in Max see the C developer's documentation.
 *
 * </p><p>
 * It is critical that the <code>release()</code> method be called
 * when a <code>MaxQelem</code> is no longer needed.
 * </p>
 * 
 * @author Topher LaFata, Ben Nevile
 */
public class MaxQelem
{
	private long _p_qelem = 0L;
	private Executable _e = null;
	private boolean hasBeenFreed = false;	
	
	/**
	 * Constructs a <code>MaxQelem</code> that executes <code>Executable</code> e.
	 * @param e the <code>Executable</code> that the <code>MaxQelem</code> will execute. 
	 */
	public MaxQelem(Executable e)
	{
		_p_qelem = _mxj_qelem_new();
		_e = e;
	}
	
	/**
	 * Constructs a <code>MaxQelem</code> that is set up to do nothing when executed.  
	 * To make anything happen the code will have to associate the <code>MaxQelem</code>
	 * with a valid <code>Executable</code> using the <code>setExecutable</code> method.
	 */
	public MaxQelem()
	{
		_p_qelem = _mxj_qelem_new();
		_e = new Executable(){
			public void execute(){};
		};
	}
	
    /**
     * Constructs a <code>MaxQelem</code> by creating a simple <code>Callback</code>
     * that executes the parameterless method in the given <code>Object</code> 
     * with the name in the given <code>String</code>.  
     * @param o the <code>Object</code> that contains the method to be executed
     * @param methodName the name of the method to execute
     */
    public MaxQelem(final Object o, String methodName) {
    	_e = new Callback(o, methodName);
    	_p_qelem = _mxj_qelem_new();
  	}
	
	/**
	 * Sets the <code>MaxQelem's</code> <code>Executable</code>.
	 * @param e the new <code>Executable</code>.
	 */
	public void setExecutable(Executable e) {
    	_e = e;
    }
    
    /**
     * Returns the <code>Executable</code> currently associated with the <code>MaxQelem</code>.
     * @return the <code>Executable</code> currently associated with the <code>MaxQelem</code>.
     */
    public Executable getExecutable()
    {
    	return _e;
    }
    

    /**
	 * This is the function that will be called when the MaxQelem is dequeued
	 * by the main thread.
	 * By default it calles the <code>execute()</code> method on its <code>Executable</code>
	 * member but can be overidden by a subclass if different behavior is desired.
	 * In normal use one would not override this function and would use the <code>Executable</code>
	 * member to accomplish whatever task is desired.
     */
    public void qfn()
    {
    	_e.execute();
    }
  

	/**
	 * Causes a <code>MaxQelem</code> to execute.  
	 * If the <code>MaxQelem</code> has already been set, it 
	 * will not be set again. 
	 * This is useful if you want to redraw the state 
	 * of some data when it changes, but not in response 
	 * to changes that occur faster than can be drawn. 
	 * A Qelem object is unset after its queue 
	 * function has been called.
	 */
	public  native void set(); 
	
	/**
	 * This function is identical to <code>set()</code>, except that the 
	 * <code>MaxQelem</code>’s function is placed at the front of the list 
	 * of routines to execute in the main thread instead of the back. 
	 * Be polite and only use <code>front()</code> 
	 * for special time-critical applications.
	 */
	public  native void front();

	
	/**
	 * Cancels a <code>MaxQelem</code>'s execution.
	 */
	public  native void unset();

    /**
     * Releases the <code>MaxQelem</code> when it's no longer needed.
     * It is <b>highly</b> recommended to call this method in your class's
     * <code>notifyDeleted</code> method to free the resources associated with a
     * <code>MaxQelem</code> when the host Max object is deleted.  
     * If you don't call this method a <code>MaxQelem</code> can execute 
     * when the host object no longer exists, and behaviour can be unpredictable.
     * It is also catastrophic to call this method and then subsequently 
     * use the <code>MaxQelem</code> again by calling the <code>set()</code> method.
     * <br>
     * The below code is an example of proper usage.
     * <br>
     * <PRE>
		public class QelemExample extends MaxObject {
		private MaxQelem q = new MaxQelem(this, "doThis");
		
		public void bang() {
			q.set();
		}
	
		private void doThis() {
			post("the Qelem is working.");
		}
	
		protected void notifyDeleted() {
			post("ouch!");
			q.release();
		}
}
</PRE>
     */
    public void release() {
    	if (!hasBeenFreed) {
    		hasBeenFreed = true;
    		unset();
    		_mxj_qelem_free();
    	}
    }
	
	
    /**
     * Called by the garbage collector on an object when garbage collection 
     * determines that there are no more references to the object.
     */
    protected void finalize() throws Throwable {
		super.finalize();
		release();
	}

	private native void _mxj_qelem_free();
	private native long _mxj_qelem_new();
}