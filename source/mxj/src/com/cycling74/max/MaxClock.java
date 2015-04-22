package com.cycling74.max;

/**
 *
 * <code>MaxClock</code> provides a way for your mxj classes to set up 
 * the execution of events in the future.  For instance, the 
 * following class creates a <code>MaxClock</code> 
 * and when it receives a bang will post a message every 500 milliseconds
 * to the Max console.
 * <br>
 * <pre>
 * public class MaxClockExample extends MaxObject() {
 * 	MaxClock cl;
 * 	
 * 	MaxClockExample() {
 * 		cl = new MaxClock(this, "runForever");
 * 	}
 * 
 * 	public void bang() {
 *  	cl.delay(500.);
 * 	}
 * 
 * 	public void runForever() {
 * 		post("forever");
 * 		cl.delay(500.);
 * 	}
 * 
 * 	public void notifyDeleted() {
 * 		post("never");
 * 		cl.release();
 * 	}
 * }
 * </pre>
 * <br>
 * It is critical that the <code>release()</code> method be called 
 * when a clock is no longer needed. 
 * 
 * 
 * @author Topher Lafata, Ben Nevile
 */
public class MaxClock
{
	private long _p_clock = 0L;
	private Executable _e = null;
	private boolean hasBeenFreed = false;
	
	/**
	 * creates a <code>MaxClock</code> that executes <code>Executable</code> e.
	 * @param e the <code>Executable</code> that the <code>MaxClock</code> will execute.
	 */
	public MaxClock(Executable e)
	{
		_p_clock = _create_max_clock();
		_e = e;
	}
	
	/**
	 * creates a <code>MaxClock</code> that is set up to do nothing when executed.  
	 * To make anything happen the code will have to associate the <code>MaxClock</code>
	 * with a valid <code>Executable</code> using the <code>setExecutable</code> method.
	 */
	public MaxClock()
	{
		_p_clock = _create_max_clock();
		_e = new Executable(){
			public void execute(){};
		};
	}
    
    /**
     * creates a <code>MaxClock</code> by creating a simple <code>Callback</code>
     * that executes the parameterless method in the given <code>Object</code> 
     * with the name in the given <code>String</code>.  
     * @param o the <code>Object</code> that contains the method to be executed
     * @param methodName the name of the method to execute
     */
    public MaxClock(final Object o, String methodName) {
    	_e = new Callback(o, methodName);
    	_p_clock = _create_max_clock();
  	}
    
    
    /**
     * gets the <code>Executable</code> currently associated with the <code>MaxClock</code>.
     * @return the <code>Executable</code> currently associated with the <code>MaxClock</code>.
     */
    public Executable getExecutable()
    {
    	return _e;
    }
    
  	/**
     * sets the <code>MaxClock</code>'s <code>Executable</code>.  For instance, 
     * the following class uses the <code>setExecutable</code> method to change
     * the output that's posted to the Max console based on integer input:
     * <br>
     * <pre>
public class SetExecutableExample extends MaxObject {
		
	private class IntHolderExe implements Executable {
		private int val;
		private MaxClock cl;
		private double delayTime;
		
		IntHolderExe(int val, MaxClock cl, double delayTime) {
			this.val = val;
			this.cl = cl;
			this.delayTime = delayTime;
		}
		
		public void execute() {
			post("the last number passed in was "+val);
			cl.delay(delayTime);
		}
	}
	
	private MaxClock cl = new MaxClock(new Executable() {
		public void execute() {
			post("no input yet.");
		}
	});
		
	public void inlet(int val) {
		cl.setExecutable(new IntHolderExe(val, cl, 500.));
	}
	
	public void bang() {
		cl.tick();
	}
	
	protected void notifyDeleted() {
		
	}
}
</pre>
     * 
     * @param e the new <code>Executable</code>.
     */
  	public void setExecutable(Executable e) {
    	_e = e;
    }
    
    
    /**
     * executes the <code>Executable</code> currently associated with the <code>MaxClock</code>.
     */
    public void tick()
    {
    	_e.execute();
    }

    
    /**
     * Releases the clock when it's no longer needed.
     * It is <b>highly</b> recommended to call this method in your class's
     * <code>notifyDeleted</code> method to free the resources associated with a
     * <code>MaxClock</code> when the host Max object is deleted.  
     * If you don't call this method a <code>MaxClock</code> can continue to operate 
     * when the host object no longer exists, and behaviour can be unpredictable.
     * It is also catastrophic to call this method and then subsequently 
     * use the <code>MaxClock</code> again by calling the <code>delay</code> method.
     * <br>
     * The below code is an example of proper usage.
     * <br>
     * <PRE>
     * 
public class ReleaseExample extends MaxObject {
	private static final double DELAY_TIME = 250.;	
	private MaxClock cl = new MaxClock(this, "doThis");
	private int i=0;
		
	public void bang() {
		cl.delay(DELAY_TIME);
	}
	
	public void doThis() {
		i++;
		post("i've printed out "+i+" numbers.");
		cl.delay(DELAY_TIME);
	}
	
	protected void notifyDeleted() {
		post("ouch!");
		cl.release();
	}
}
</PRE>
     */
    public void release() {
    	if (!hasBeenFreed) {
    		unset();
    		_free_clock();
    		hasBeenFreed = true;
    	}
    }
    

	/**
	 * Set up a clock tick for some time in the future.  Cancels any previous
	 * <code>delay</code> calls that have not yet executed.
	 * @param time milliseconds until the next clock tick (execution of the 
	 * <code>Executable</code>.)
	 */
	public  native void delay(double time); 
	
	/**
	 * Unsets the clock.  Cancels any previous
	 * <code>delay</code> calls that have not yet executed.
	 */
	public  native void unset();
	
	
	/**
	 * Gets the current logical time of the scheduler.
	 * @return the current logical time of the scheduler in milliseconds
	 */
	public static native double getTime();
	
	
    /**
     * Called by the garbage collector on an object when garbage collection 
     * determines that there are no more references to the object.
     */
    protected void finalize() throws Throwable
	{
		super.finalize();
		release();
	}
    
    
	private native void _free_clock();
	private native long _create_max_clock();

}