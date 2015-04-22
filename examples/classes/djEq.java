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

public class djEq extends MSPPerformer
{
	//biquads for each EQ band
	private biquad _loshelf;
	private biquad _lomid;
	private biquad _himid;
	private biquad _hishelf;

	//gain range in db for all bands
	public int mingain = -28;
	public int maxgain = 8;

	public double lofreq;
	public double lomidfreq;
	public double himidfreq;
	public double hifreq;

	private static final String[] INLET_ASSIST = new String[]{
		"input(sig)",
		"lo level (0-127)",
		"lo mid level (0-127)",
		"hi mid level (0-127)",
		"hi level (0-127)"
	};
	private static final String[] OUTLET_ASSIST = new String[]{
		"output (sig)"
	};


	public djEq(float gain)
	{
		declareInlets(new int[]{SIGNAL,DataTypes.ALL,DataTypes.ALL,DataTypes.ALL,DataTypes.ALL});
		declareOutlets(new int[]{SIGNAL});
		setInletAssist(INLET_ASSIST);
		setOutletAssist(OUTLET_ASSIST);		

		declareAttribute("mingain");
		declareAttribute("maxgain");
		declareAttribute("lofreq",null,"setLoFreq"); 
		declareAttribute("lomidfreq",null,"setLoMidFreq");
		declareAttribute("himidfreq",null,"setHiMidFreq");
		declareAttribute("hifreq",null,"setHiFreq");

		//set default center frequencies for EQ bands. These are arbitrary values
		lofreq    = 60;
		lomidfreq = 500;
		himidfreq = 2200;
		hifreq   = 10000;
	
		_loshelf = new biquad(biquad.LOWSHELF,lofreq,2.0,0.0);
		_lomid   = new biquad(biquad.PEAK,lomidfreq,2.0,0.0);
		_himid   = new biquad(biquad.PEAK,himidfreq,2.0,0.0);
		_hishelf = new biquad(biquad.HISHELF,hifreq,2.0,0.0);

	}
    
	public void setLoFreq(double freq)
	{
		_loshelf.cutoff = freq;
		_loshelf.updateCoeffs();		
	}

	public void setLoMidFreq(double freq)
	{
		_lomid.cutoff = freq;
		_lomid.updateCoeffs();		
	}

	public void setHiMidFreq(double freq)
	{
		_himid.cutoff = freq;
		_himid.updateCoeffs();		
	}

	public void setHiFreq(double freq)
	{
		_hishelf.cutoff = freq;
		_hishelf.updateCoeffs();		
	}



	public void inlet(float f)
	{
		//map incoming values into approprite range from 0-127 to MIN/MAX gain
		//where an incoming value of 64 is 0. This allows the EQ knobs to be at 12
		//o'clock for 0 db gain
		f = f - 64;
		switch(getInlet())
		{
			
			case 1:
				if(f >= 0.)
					_loshelf.gain = scale(f,0,64,0,maxgain);
				else if (f < 0)
					_loshelf.gain = scale(f,-64,0,mingain,0);
	
				_loshelf.updateCoeffs();
				break;
			case 2:
				if(f >= 0.)
					_lomid.gain = scale(f,0,64,0,maxgain);
				else if (f < 0)
					_lomid.gain = scale(f,-64,0,mingain,0);
	
				_lomid.updateCoeffs();
				break;
			case 3:
				if(f >= 0.)
					_himid.gain = scale(f,0,64,0,maxgain);
				else if (f < 0)
					_himid.gain = scale(f,-64,0,mingain,0);
	
				_himid.updateCoeffs();
				break;
			case 4:
				if(f >= 0.)
					_hishelf.gain = scale(f,0,64,0,maxgain);
				else if (f < 0)
					_hishelf.gain = scale(f,-64,0,mingain,0);
	
				_hishelf.updateCoeffs();
				break;
			default:
				break;
		}
	}

	public void dspsetup(MSPSignal[] ins, MSPSignal[] outs)
	{
			//we need to call dspsetup on each of our performers so
			//that they are properly initialized	
			_loshelf.dspsetup(ins,outs);
			_lomid.dspsetup(ins,outs);
			_himid.dspsetup(ins,outs);
			_hishelf.dspsetup(ins,outs);
	}

	public void perform(MSPSignal[] ins, MSPSignal[] outs)
	{
		int i;
		float[] in = ins[0].vec;
		float[] out = outs[0].vec;
		
		//Notice how we swap the signal arrays so that we can chain a bunch
		//of performers together. We could also use MSPSignal.dupclean() to get
		//temporary MSPSignal arrays if we wanted unique MSPSignal arrays for
		//each connection in the chain
		_loshelf.perform(ins,outs);
		_lomid.perform(outs,ins);
		_himid.perform(ins,outs);
		_hishelf.perform(outs,ins);

		for(i = 0; i < in.length;i++)
		{
			out[i] = in[i];  	
		}
	}
	
	//generic scaling function
	private static float scale(float in, float from1, float from2, float to1, float to2)
	{	
		float ret = (in - from1) * (to2-to1)/(from2 - from1) + to1;
		return ret;
	}

}




