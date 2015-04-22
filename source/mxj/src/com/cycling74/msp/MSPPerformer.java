package com.cycling74.msp;
import java.lang.reflect.Method;

/**
 * Alternative base class for a Java signal processing object.Subclassing MSPPerformer is 
 * somewhat simpler than subclassing MSPObject directly. Subclass this directly or
 * indirectly to create your own Java signal processing objects for use in Max.
 * See the mxj~ examples directory in the java-doc directory of your Max
 * install for more sample code.
 *
 * See also 'WritingMaxExternalsInJava.pdf' for a more detailed discussion about writing
 * Max signal processing classes in Java.
 *
 *@author Topher LaFata
 */
public abstract class MSPPerformer extends MSPObject implements MSPPerformable
{
	private Method _sp = null;
	private static final Class MSP_PERFORMER_CLZ = __load_perf_clz();
	
	public Method dsp(MSPSignal[] in, MSPSignal[] out)
	{
		try{
        	_sp = MSP_PERFORMER_CLZ.getDeclaredMethod("superperform",new Class[]{MSP_SIGNAL_ARRAY_CLZ,MSP_SIGNAL_ARRAY_CLZ});
        }catch(NoSuchMethodException nsme)
            {
            	post("(mxj~) perform method superperform was not found in class "+this.getClass().getName());
       			
            }
		dspsetup(in,out);
		return _sp;	
	}

	private void superperform(MSPSignal[] in, MSPSignal[] out)
	{

		perform(in,out);		
	}
	/**
	 * The dspsetup method is called once when the MSP signal compiler is building the dsp chain
	 * for the patch which contains your MSPPerformer instance. You are passed two arrays of 
	 * MSPSiganl objects corresponding to the SIGNAL inlets/outlets you declared in your 
	 * declareInlets/decalreOutlets calls in your constructor.
	 *
	 * You can interrogate any of these MSPSiganl instances for meta information about the 
	 * current dsp context including sampling rate, sample vector size etc. 
     * 
	 * By default this method does nothing.
	 *
	 * @see com.cycling74.msp#MSPSignal
     * @param sigs_in array of MSPSiganl objects corresponding to the type SIGNAL inlets declared in 
	 * your constructor.
	 * @param sigs_out array of MSPSiganl objects corresponding to the type SIGNAL outlets declared in 
	 * your constructor.
	 */
	public void dspsetup(MSPSignal[] sigs_in,MSPSignal[] sigs_out)
	{
	
	}
	/**
	* The perform method is called continuosly as part of the MSP dsp chain to process sample vectors
	* after the dspsetup method is called and until the MSP dsp chain is stopped.This is the workhorse
	* of your signal processing class.
	* @param sigs_in array of MSPSiganl objects corresponding to the type SIGNAL inlets declared in 
	* your constructor.
	* @param sigs_out array of MSPSiganl objects corresponding to the type SIGNAL outlets declared in 
	* your constructor.
	*/
	public abstract void perform(MSPSignal[] sigs_in, MSPSignal[] sigs_out);

	private static Class __load_perf_clz()
	{
	
		try{
		  return Class.forName("com.cycling74.msp.MSPPerformer");
		}catch(Exception e)
		{
			return null;
		}
	}
}