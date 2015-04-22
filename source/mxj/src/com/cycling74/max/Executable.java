
package com.cycling74.max;


/**
 * Executable provides a common interface for classes that
 * need a method to execute. This is similar to passing a function
 * pointer in C.
 * If a Class implements Executable, then it must have an 
 * execute() method defined.  Classes like MaxClock and MaxQelem
 * require an Executable as a construction argument, and so can 
 * rely on the execute() method being present.  The Java programmer
 * can create a class that implement Executable and use the execute() 
 * method to define what the Class will do when it comes time to execute.
 * <br>
 * The code below shows how you can make any class into an Executable, and
 * have it pass a reference to itself in a MaxClock constructor.
 * <pre>
   <code>
	public class ExeExample extends MaxObject implements Executable {
		
		private static final double DELAY_TIME = 500.;
	  	private int counter = 0;
	  	private MaxClock clock;
	  
	  	ExeExample() {
	  		clock = new MaxClock(this);
	 	}
	 	
	  	public void bang() {
	  		clock.delay(DELAY_TIME);	
	 	}
	  	public void stop()
	  	{
	  		clock.unset();
	  	}
	  	public void execute() {	  
	  		counter++;
	  		post("this method has executed " + counter + " times.");
	  		clock.delay(DELAY_TIME);
		}
		protected void notifyDeleted()
		{
			clock.release();
		}
	}
 </code>
 </pre>
 * 
 * @author Topher Lafata
 */

public abstract interface Executable
{
	/**
	 * the method to execute.
	 */
	public abstract void execute();
}


	
									