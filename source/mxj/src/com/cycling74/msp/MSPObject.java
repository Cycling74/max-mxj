package com.cycling74.msp;

import com.cycling74.max.*;
import java.lang.reflect.Method;

/**
 * Base class for a Java signal processing object.  Subclass this directly or
 * indirectly to create your own Java signal processing objects for use in Max.
 * See the mxj~ examples directory in the java-doc directory of your Max
 * install for more sample code.
 *
 * See also 'WritingMaxExternalsInJava.pdf' for a more detailed discussion about writing
 * Max signal processing classes in Java.
 <pre>
	//This example adds two signals together. If only one signal is connected
	//it adds the last floating point or integer value received to that signal.
	//If neither signal is connected it outlets the last floating point or integer
	//value received.
	import com.cycling74.max.*;
	import com.cycling74.msp.*;
	import java.lang.reflect.Method;
	public class sigadd extends MSPObject
	{
		private float addend;
		private Method _p1;
		private Method _p2;
		private Method _p3;
		public sigadd()
		{
			this(0);
		}

		public sigadd(float f)
		{
			addend = f;
			declareInlets(new int[]{SIGNAL,SIGNAL});
			declareOutlets(new int[]{SIGNAL});
			_p1 = getPerformMethod(Òp1Ó);
			_p2 = getPerformMethod(Òp2Ó);
			_p3 = getPerformMethod(Òp3Ó);
			_p4 = getPerformMethod(Òp4Ó);
		}
		public void inlet(float f)
		{
			addend = f;
		}
		public Method dsp(MSPSignal[] in, MSPSignal[] out)
		{
			if(in[0].connected && !in[1].connected)
				return _p1;
			else if(!in[0].connected && in[1].connected)
				return _p2;
			else if(in[0].connected && in[1].connected)
				return _p3;
			else
				return _p4;
		}
		public void p1(MSPSignal[] in, MSPSignal[] out)
		{
			float[] in1 = in[0].vec;
			float[] o = out[0].vec;
			int vec_size = in[0].n;
			int i;
			//2nd signal inlet is not connected
			for(i = 0;i < vec_size;i++)
				o[i] = in1[i] + addend;
		}
		public void p2(MSPSignal[] in, MSPSignal[] out)
		{
			float[] in2 = in[1].vec;
			float[] o = out[0].vec;
			int vec_size = in[0].n;
			int i;
			//1st signal inlet is not connected
			for(i = 0;i < vec_size;i++)
				o[i] = in2[i] + addend;
		}
		public void p3(MSPSignal[] in, MSPSignal[] out)
		{
			float[] in1 = in[0].vec;
			float[] in2 = in[1].vec;
			float[] o = out[0].vec;
			int vec_size = in[0].n;
			int i;
			//both signals inlet are connected
			for(i = 0;i < vec_size;i++)
				o[i] = in1[i] + in2[i];
		}
		public void p4(MSPSignal[] in, MSPSignal[] out)
		{
			//neither signal inlet is connected
			for(i = 0;i < vec_size;i++)
				o[i] = addend;
		}
	}
	</pre>
 *
 * @author Topher LaFata
 */

public abstract class MSPObject extends MaxObject
{
	private boolean _z_no_inplace = false;
	//signal inlet/outlet type
	/**
     * A constant for use with declareInlets() and declareOutlets() to enable the creation of MSP signal
	 * inlets and outlets.
     */
	public static final int SIGNAL = 32;
	/**
     * A convenience constant initialized to the contain the Class Object for MSPSignal[]
     */
	public static final Class MSP_SIGNAL_ARRAY_CLZ = (new MSPSignal[0]).getClass();
	
	/**
	 * Since it is abstract, the dsp method must be implemented by your MSPObject subclass.
     * It provides the mechanism to notify the mxj~ object which class method it should be 
	 * calling to process sample vectors. The Method object it returns must have the 
	 * signature:
	 * <pre>
	 *		private void myPerformMethod(MSPSignal[] inlets, MSPSignal[] outlets)
     * </pre>
	 * The dsp method is called once when the MSP signal compiler is building the dsp chain
	 * for the patch which contains your MSPObject instance. You are passed two arrays of 
	 * MSPSiganl objects corresponding to the SIGNAL inlets/outlets you declared in your 
	 * declareInlets/decalreOutlets calls in your constructor.
	 *
	 * You can interrogate any of these MSPSiganl instances for meta information about the 
	 * current dsp context including sampling rate, sample vector size etc. 
     * @see com.cycling74.msp#MSPSignal
     * @param inlets array of MSPSiganl objects corresponding to the type SIGNAL inlets declared in 
	 * your constructor.
	 * @param outlets array of MSPSiganl objects corresponding to the type SIGNAL outlets declared in 
	 * your constructor.
	 * @return instance of java.lang.reflect.Method specifying which method you would like to be called
	 * to process samples in the current MSP siganl processing context.
	 */
	public abstract Method dsp(MSPSignal[] inlets, MSPSignal[] outlets);
	
	/**
     * Helper to aide in the creation of java.lang.reflect.Method instances 
	 * which could be returned by your dsp method. The name arg must be the name
	 * of a method in your class with the following signature:
	 *  <pre>
	 *		private void myPerformMethod(MSPSignal[] inlets, MSPSignal[] outlets)
     * </pre>
     * @param name the name of the method you would like returned as a 
	 * java.lang.reflect.Method instance.
     * @return java.lang.reflect.Method instance of class method named name or null
	 * if the method does not exist or does not have a valid signature.
	 */
	protected Method getPerformMethod(String name)
	{
		Method p = null;
		try{
        	p = getClass().getDeclaredMethod(name,new Class[]{MSP_SIGNAL_ARRAY_CLZ,MSP_SIGNAL_ARRAY_CLZ});
        	return p;
        }catch(NoSuchMethodException nsme)
            {
            	post("(mxj~) perform method "+name+" was not found in class "+this.getClass().getName());
       			
            }
        return null; 
		
	}
	
	/** 
	* If you set this to true, the compiler will guarantee that all the signal vectors
	* passed to your object will be unique. It is common that one or more of the output
	* vectors your object will use in its perform method will be the same as one or more of
	* its input vectors. Some objects are unable to handle this restriction; typically, this
	* occurs when an object has pairs of inputs and outputs and writes an entire output on
	* the basis of a single input before moving on to another input-output pair.
	*/
	protected void setNoInPlace(boolean b)
	{
		_z_no_inplace = b;
	}
	
	/**
	* Called by Max when the dsp chain is started or stopped. By default it does nothing.
	* Override this method if you would like to do something in response to the msp dsp 
	* processing chain being stopped or started.
	* @param b The current state (on/off) of the MSP dsp chain.
	*/
	protected void dspstate(boolean b)
	{
	
	}
}