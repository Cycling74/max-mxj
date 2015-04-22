package com.cycling74.msp;

/**
* Interface for specifying that your object supports the same
* naming convention for dsp and perform routines as MSPPerformer.
* @see com.cycling74.msp#MSPPerformer
*/
public interface MSPPerformable
{
	
	public abstract void dspsetup(MSPSignal[] sigs_in,MSPSignal[] sigs_out);
	public abstract void perform(MSPSignal[] sigs_in,MSPSignal[] sigs_out);

}