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
import java.lang.reflect.*;

//phaser.java - a simple 6 stage phaser
//
//adapted from code in the musicdsp archive. http://www.musicdsp.org by
//"Ross Bencina" <rbencina@hotmail.com> for mxj~ by turtleman
//
//perform routine could be unrolled for better performance

public class phaser extends MSPObject implements MSPPerformable
{

	private static final float F_PI = 3.1314159f;

	private Method _p   = null;
	private double _sr  = 0;

	AllPass[] _alps;

	public float minrange;//hz
	public float maxrange;//hz
	public float depth;//0-1
	public float rate;//hz ??
	public float feedback;//0-1

	float _dmin;
	float _dmax;
	float _lfo_phase;
	float _lfo_inc;
	float _zm1;

 	
	private static final String[] INLET_ASSIST = new String[]{
		"input (sig)"
	};
	private static final String[] OUTLET_ASSIST = new String[]{
		"output (sig)"
	};
	
	public phaser()
	{
		declareInlets(new int[]{SIGNAL});
		declareOutlets(new int[]{SIGNAL});

		setInletAssist(INLET_ASSIST);
		setOutletAssist(OUTLET_ASSIST);

		declareAttribute("minrange");
		declareAttribute("maxrange");
		declareAttribute("depth");
		declareAttribute("rate");
		declareAttribute("feedback");

		_p = getPerformMethod("p");	

		_alps = new AllPass[6];
		for(int i = 0; i < _alps.length;i++)
			_alps[i] = new AllPass();

		minrange = 440;
		maxrange = 1600;
		rate = 0.5f;
		feedback = 0.7f;
		depth   = 1.0f;

		_lfo_phase = 0.0f;
		_zm1 = 0.0f;		
		_sr = 0;

	
	}
    

	public Method dsp(MSPSignal[] ins, MSPSignal[] outs)
	{
		_sr = ins[0].sr;	
		return _p;
	}

	public void p(MSPSignal[] ins, MSPSignal[] outs)
	{
		
		int i;
		float[] in = ins[0].vec;
		float[] out = outs[0].vec;
		
		_dmin = minrange / (float)(_sr/2);
		_dmax = maxrange / (float)(_sr/2);
		_lfo_inc = 2 * F_PI * (float)(rate/_sr);

		for(i = 0; i < in.length;i++)
		{

			float d = _dmin + (_dmax - _dmin) * (((float)Math.sin( _lfo_phase ) + 1.f)/2.f);
  			_lfo_phase += _lfo_inc;
        	if( _lfo_phase >= F_PI * 2.f )
        		_lfo_phase -= F_PI * 2.f;

       		//update filter coeffs
       		 for( int c=0; c<_alps.length; c++ )
        		_alps[c].delay( d );

        	//calculate output
        	float y = 	_alps[0].update(
        				 _alps[1].update(
           		           _alps[2].update(
           		            _alps[3].update(
           		             _alps[4].update(
           		              _alps[5].update( in[i] + _zm1 * feedback ))))));
        	_zm1 = y;
        	out[i] = in[i] + y * depth;
			/*do something*/ 	

		}
	}
	

	class AllPass
	{
		float _a1;
		float _zm1;

		AllPass()
		{
			_a1  = 0;
			_zm1 = 0;
		}

        void delay( float delay ){ //sample delay time
        	_a1 = (1.f - delay) / (1.f + delay);
        }

		float update(float samp_in)
		{
			float y = samp_in * - _a1 + _zm1;
			_zm1 = y * _a1 + samp_in;	
			return y;	
		}

	}

//MSPPerformable Interface/////////////////////////////////////////////////////////////
	public void perform(MSPSignal[] ins, MSPSignal[] outs)
	{
		p(ins,outs);
	}

	public void dspsetup(MSPSignal[] ins, MSPSignal[] outs)
	{
		dsp(ins,outs);
	}
	//End MSPPerformable Interface
}





