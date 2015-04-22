/*
	Copyright (c) 2012 Cycling '74

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
	and associated documentation files (the "Software"), to deal in the Software without restriction, 
	including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
	and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies 
	or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

import com.cycling74.max.*;
import com.cycling74.msp.*;


public class envfollow extends MSPPerformer
{

	private float threshold;
	private float attack;
	private float release;
	private double attack_coef;
	private double release_coef;
	private double envelope;
	private double sr;

	private static final String[] INLET_ASSIST = new String[]{
		"input (sig)"
	};

	private static final String[] OUTLET_ASSIST = new String[]{
		"env output (sig)"
	};

    public envfollow()
    {
		attack = 100;
		release = 100;
		threshold = 0.5F;

		declareInlets(new int[]{SIGNAL});
		declareOutlets(new int[]{SIGNAL});
		
		declareAttribute("attack",null,"setAttack");
		declareAttribute("threshold",null,"setThreshold");
		declareAttribute("release",null,"setRelease");

	}
    
	public void setAttack(float ms)
	{
		attack = ms;
		attack_coef = Math.exp(Math.log(0.01)/( attack * sr * 0.001));
	}

	public void setThreshold(float th)
    {
	    threshold = th;
	}
    public void setRelease(float ms)
    {
	    release = ms;
		release_coef = Math.exp(Math.log(0.01)/( release * sr * 0.001));
	}
    
    public void dspsetup(MSPSignal[] in , MSPSignal[] out)
    {
	    sr = in[0].sr;
		attack_coef = Math.exp(Math.log(0.01)/( attack * sr * 0.001));
		release_coef = Math.exp(Math.log(0.01)/( release * sr * 0.001));
		envelope = 0.0;
	}
    
	public void perform(MSPSignal[] ins, MSPSignal[] outs) 
	{
		float[] in  = ins[0].vec;
		float[] out = outs[0].vec;
		float tmp;	
		for(int i = 0; i < in.length;i++)
		{
			
			tmp = in[i];
			if(tmp < 0)
				tmp = -tmp;
			if(tmp >= threshold)
			{	
				if(tmp > envelope)
					envelope = attack_coef * (envelope - tmp) + tmp;
				else
					envelope = release_coef * (envelope - tmp) + tmp;
				out[i] =(float) envelope;	
			}
			else
			{
				out[i] = 0;
			}		
		}

	}
    
}



