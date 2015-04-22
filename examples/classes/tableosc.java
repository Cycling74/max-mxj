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

public class tableosc extends MSPObject implements MSPPerformable
{
	//truncating wavetable oscillator.

	private static final double TWO_PI = Math.PI * 2;

	public float freq = 0.f;
	public float phase = 0.f;

	private float[] _table;
	private int L; //table length
	private double _sr;//sampling rate
	private float _index;//current index
	private float _inc;//current increment
	private static final String[] INLET_ASSIST = new String[]{
		"freq (sig/float)","phase 0-1 (sig/float)"
	};
	private static final String[] OUTLET_ASSIST = new String[]{
		"output (sig)"
	};
	
	//perform routines
	private Method _p1    = null;
	private Method _p2   = null;
	private Method _p3   = null;
	private Method _p4   = null;

	public tableosc(float[] usertable)
	{
		post("usertable const called");
		declareInlets(new int[]{SIGNAL,SIGNAL});
		declareOutlets(new int[]{SIGNAL});

		setInletAssist(INLET_ASSIST);
		setOutletAssist(OUTLET_ASSIST);

		_p1 = getPerformMethod("p1");
		_p2 = getPerformMethod("p2");
		_p3 = getPerformMethod("p3");
		_p4 = getPerformMethod("p4");

		int l = usertable.length;
		L     = (int) usertable[l-1];
		phase = usertable[l-2];
		freq  = usertable[l-3];	
		
		_index 	= 0;
		_inc 	= 0;
		_sr 	= 0;

		float[] hamp = new float[l-3];
		for(int i = 0; i < hamp.length;i++)
			hamp[i] = usertable[i];
		_make_user_table(hamp);
	}

	public tableosc(String type, float fr, float ph, int size)
	{
		declareInlets(new int[]{SIGNAL,SIGNAL});
		declareOutlets(new int[]{SIGNAL});

		setInletAssist(INLET_ASSIST);
		setOutletAssist(OUTLET_ASSIST);

		_p1 = getPerformMethod("p1");
		_p2 = getPerformMethod("p2");
		_p3 = getPerformMethod("p3");
		_p4 = getPerformMethod("p4");

		if(size <= 0)
			L = 512;
		else
			L = size;

		freq	= fr;
		phase 	= ph;

		_index 	= 0;
		_inc 	= 0;
		_sr 	= 0;

		_make_sin_table();//default to SINE wave
	}
    

	public void table(float[] harmonic_amplitudes)
	{
		_make_user_table(harmonic_amplitudes);
	}

	public void table(String type)
	{
		String t = type.toUpperCase();
		if(t.equals("SIN"))
			_make_sin_table();
		else if(t.equals("SQR"))
			_make_sqr_table();
		else if(t.equals("TRI"))
			_make_tri_table();
		else if(t.equals("SAW"))
			_make_saw_table();
		else
		{
			post("(mxj~ tableosc)Unknown table type "+type+" defaulting to SIN table");	
			_make_sin_table();
		}	
	}

	public void inlet(float f)
	{
		switch(getInlet())
		{
			case 0:
				freq(f);
				break;
			case 1:
				phase(f);
				break;
		}
	}

	public void freq(float f)
	{
		freq = f;
		_inc = (float) (freq * L / _sr); 		
	}

	public void phase(float p)
	{
		phase = p;
	}

	private void _make_sin_table()
	{
		if(_table == null)
			_table = new float[L];

		for(int i = 0; i < L;i++)
			_table[i] = (float)Math.cos(TWO_PI * i / L);
	}

	private void _make_sqr_table()
	{
		if(_table == null)
			_table = new float[L];

		float max = 0;
		//zero table
		for(int i = 0; i < L;i++)
			_table[i] = 0;

		for(int h = 1; h <= 9; h +=2)
		{
			for(int i = 0; i < L;i++)
			{
				_table[i] += (float)((1.0/h) * Math.sin(TWO_PI*h*i/L));
				if(Math.abs(_table[i]) > max)
					max = _table[i];				
			}

		}
		
		//normalize
		for(int i = 0; i < L;i++)
			_table[i] /= max;
	}

	private void _make_tri_table()
	{
		float max = 0;
		if(_table == null)
			_table = new float[L];

		float inc = (float)(2.0 /( L / 2.0));
		float val = -1;			
		for(int i = 0; i < L;i++)
		{
	
			_table[i] = val;
			if(i < L/2)
				val	+= inc;
			else
				val -= inc; 				
		}
		
		//normalize
		//for(int i = 0; i < L;i++)
		//	_table[i] /= max;
	}

	private void _make_saw_table()
	{
		if(_table == null)
			_table = new float[L];
		float max = 0;
	
		//zero table
		for(int i = 0; i < L;i++)
			_table[i] = 0;
		for(int h = 1; h <= 9; h ++)
		{
			for(int i = 0; i < L;i++)
			{
				_table[i] += (float)((1.0/h) * Math.sin(TWO_PI*h*i/L));
				if(Math.abs(_table[i]) > max)
					max = _table[i];				
			}

		}
		//normalize
		for(int i = 0; i < L;i++)
			_table[i] /= max;
	}

	private void _make_user_table(float[] harmonics)
	{
		float max = 0;

		if(_table == null)
			_table = new float[L];	
		else
		{
			//zero table
			for(int i = 0; i < L;i++)
				_table[i] = 0;
		}

		for(int h = 0; h < harmonics.length; h++)
		{
			for(int i = 0; i < L;i++)
			{
				_table[i] += (float)(harmonics[h] * Math.sin(TWO_PI*(h+1)*i/L));
				if(Math.abs(_table[i]) > max)
					max = _table[i];				
			}		
		}

		//normalize
		for(int i = 0; i < L;i++)
			_table[i] /= max;	
	}

	//reset osc to beginning of cycle
	public void reset()
	{
		_index = 0;
	}

	public Method dsp(MSPSignal[] ins, MSPSignal[] outs)
	{
		_sr = outs[0].sr;
		_inc = (float) (freq * L / _sr); 		

		if(ins[0].connected && ins[1].connected)
			return _p4;
		else if(!ins[0].connected && ins[1].connected)
			return _p3;
		else if(ins[0].connected && !ins[1].connected)
			return _p2;
		else
			return _p1;
	}

	public void p1(MSPSignal[] ins, MSPSignal[] outs)
	{		
		int i;
		float[] out	= outs[0].vec;
		float p_offset = phase * (L - 1);

		for(i = 0; i < out.length;i++)
		{
			out[i] = _table[(int)(_index + p_offset) % L];		
			_index += _inc;
		}		
		_index %= L;
	}

	//freq signal inlet connected
	public void p2(MSPSignal[] ins, MSPSignal[] outs)
	{
		int i;
		float[] f   = ins[0].vec;//freq	
		float[] out	= outs[0].vec;

		float p_offset = phase * (L - 1);
	
		for(i = 0; i < out.length;i++)
		{
			out[i] = _table[(int)(_index + p_offset) % L];		
			_index += f[i] * L / _sr;
		}		
		_index %= L;
	}

	
	//phase signal inlet connected
	public void p3(MSPSignal[] ins, MSPSignal[] outs)
	{
		int i;
		float[] f   = ins[0].vec;//sig freq
		float[] ph  = ins[1].vec;//sig phase	
		float[] out	= outs[0].vec;

		for(i = 0; i < out.length;i++)
		{
			out[i] = _table[(int)(_index + ph[i] * (L - 1)) % L];		
			_index += _inc;
		}		
		_index %= L;
	}	
	
	//freq and phase signal inlets connected
	public void p4(MSPSignal[] ins, MSPSignal[] outs)
	{
		int i;
		float[] f   = ins[0].vec;//sig freq
		float[] ph  = ins[1].vec;//sig phase	
		float[] out	= outs[0].vec;

		for(i = 0; i < out.length;i++)
		{
			out[i] = _table[(int)(_index + ph[i] * (L - 1)) % L];		
			_index += f[i] * L / _sr;
		}		
		_index %= L;
	}	

	//MSPPerformable Interface
	public void perform(MSPSignal[] ins, MSPSignal[] outs)
	{
		p1(ins,outs);
	}

	public void dspsetup(MSPSignal[] ins, MSPSignal[] outs)
	{
		dsp(ins,outs);
	}
	//End MSPPerformable Interface
}








