package com.cycling74.msp;

/**
 * MSPSignal is the Java object representation of the sample vector used
 * by MSP processing data at audio rate. It also contains meta data about the
 * current MSP siganl processing context as well 
 */
public class MSPSignal
{
	/**
     * The current vector sample data.
     */
    public final float[] vec;//float array containg sample vector
   /**
   * Sampling rate of the current MSP dsp context.
   */
	public double sr;//sample rate
	/**
	* Size of sample vectors in the current MSP dsp context.This
	* is currently equivalent to vec.length.
	*/
    public int n;//vector_size
	/**
	* In your dsp and perform routines you are passed an array of MSPSiganl
	* instances corresponding to the SIGNAL inlets and outlets you declared
	* in your constructor. This field contains the number of signals currently
	* connected to that inlet/outlet.
	*/
    public short cc;//connection count
	/**
	* In your dsp and perform routines you are passed an array of MSPSiganl
	* instances corresponding to the SIGNAL inlets and outlets you declared
	* in your constructor. This field is true if any signals are connected to
	* that inlet/outlet and false if the inlet/outlet has no signal connected
	* to it.
	*/
    public boolean connected;//connected flag
	/**
	* Constructor for creating your own MSPSignal instances. This can be useful
	* when you wish to pass an MSPPerformer or MSPObject sample vectors that you
	* are generating programatically in java or otherwise call MaxObject and MSPPerformer
	* dsp or perform routines in an arbitrary context. 
	*/	
    public MSPSignal(float[] vec_,double sr_,int n_, short cc_)
    {

    	vec = vec_;
    	sr  = sr_;
    	n   = n_;
    	cc  = cc_;
    	if(cc > 0)
    		connected = true;
    	else
    		connected = false;
    }   
    /**
	*Return an exact and unique copy of this instance of MSPSignal.
	*/
    public MSPSignal dup()
    {
    	float[] v;
    	v = new float[n];    		
    	System.arraycopy(vec,0,v,0,n);
    	return new MSPSignal(v,sr,n,cc);
    }
	/**
	*Return an exact and unique copy of this instance of MSPSignal with
	* a zeroed out sample vector.
	*/
    public MSPSignal dupclean()
    {
    	return new MSPSignal(new float[n],sr,n,cc);
    }
	/**
	* Return a unique instance of this MSPSignal pointing to the same
	* sample vector and having identical sample rate, vector size, connection count etc.
	*/
	public MSPSignal alias()
	{
		return new MSPSignal(vec,sr,n,cc);
	}
	/**
	*Get a printable representation of this MSPSignal instance.
	*/
    public String toString()
	{
		return new String(" vec: "+vec+" sr: "+sr+" n: "+n+" cc: "+cc+" connected: "+connected);
	}
      
}