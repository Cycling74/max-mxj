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

public class fbDelay extends MSPObject implements MSPPerformable
{
    
    Method _p  = null;
    Method _p2 = null;
    Method _p3 = null;

    private float delay;
    private int _maxdelaytime;//ms
    private float feedback;
    private int _write_index;
    private int _read_index;
    private int _bufsize;
    private float[] _delay_line;
    private float _last_sample = 0; 
    
    private static final String[] INLET_ASSIST = new String[]{
		"input (sig)",
		"delay (float/sig)",
		"feedback (float/sig)"
    };

    private static final String[] OUTLET_ASSIST = new String[]{
		"output (sig)",
    };

    
    public fbDelay()
    {
		this(5000);//default to 5 seconds max delay
    }

    public fbDelay(int max_delay_time)
    {
		declareInlets(new int[]{SIGNAL,SIGNAL,SIGNAL });
		declareOutlets(new int[]{SIGNAL});

		setInletAssist(INLET_ASSIST);
		setOutletAssist(OUTLET_ASSIST);

		declareAttribute("feedback");
		declareAttribute("delay",null,"setDelay");

		delay = 0;
		feedback = 0;

		_maxdelaytime = max_delay_time;
		_write_index  = 0;
		_read_index  = 0;

		_p = getPerformMethod("p");
		_p2 = getPerformMethod("p2");
		_p3 = getPerformMethod("p3");

    }

    public void setDelay(float ms)
    {

		delay = ms;
		_read_index = (int)(_write_index  - (delay * 44.1));
		if(_read_index < 0)
		    _read_index = _read_index +  _bufsize;
  	 }

	public void inlet(float f)
	{
		switch(getInlet())
		{
			case 1:
				setDelay(f);
				break;
			case 2:
				feedback = f;
				break;
			default:
				break;
		}
	}
 
    public Method dsp(MSPSignal[] in, MSPSignal[] out)
    {
	
		_bufsize  =(int) ((in[0].sr / 1000.0)  * _maxdelaytime);
		_delay_line   = new float[_bufsize];

		//we set read index again here in case someone disconnected the signal
		//inlet for delay time ..

		_read_index = (int)(_write_index  - (delay * 44.1));
		if(_read_index < 0)
		    _read_index = _read_index +  _bufsize;

		if(in[1].connected && !in[2].connected)//signal rate delay time
		    return _p2;
		else if(in[1].connected && in[2].connected)//signal rate delay time and feedback
		    return _p3;
		else
		    return _p;
	
    }	 

    //no signal inputs for delay time or feedback 
    private void p(MSPSignal[] sigin,MSPSignal[] sigout)
    {

		float[] in  = sigin[0].vec;//signal input
		float[] out = sigout[0].vec;//output
		float is;

		for(int i = 0; i < in.length;i++)
		{
			is = in[i] + _last_sample;
			_delay_line[_write_index] = is;
			out[i] = _delay_line[_read_index]; 
			_last_sample = _delay_line[_read_index] * feedback;
			_write_index++;
			_read_index++;
			if(_write_index >= _bufsize)
				_write_index = 0;
			if(_read_index >= _bufsize)
				_read_index = 0;
		}
    }
    
    //signal input for delay time but not feedback
    private void p2(MSPSignal[] sigin, MSPSignal[] sigout)
    {
		float[] in   = sigin[0].vec;//signal input
		float[] in2  = sigin[1].vec;//signal delay time
		float[] out  = sigout[0].vec;//output
		float is;
		for(int i = 0;i < in.length;i++)
		{
			_read_index = (int)(_write_index - in2[i] * 44.1);
			if(_read_index < 0)
			    _read_index = _read_index + _bufsize;
			is = in[i] + _last_sample;
			_delay_line[_write_index] = is;
			out[i] = _delay_line[_read_index]; 
			_last_sample = _delay_line[_read_index] * feedback;
			_write_index++;
			if(_write_index >= _bufsize)
				_write_index = 0;
			if(_read_index >= _bufsize)
				_read_index = 0;
	    }
    }

    //signal input for delay time and feedback
    private void p3(MSPSignal[] sigin, MSPSignal[] sigout)
    {
		float[] in   = sigin[0].vec;//signal input 
		float[] in2  = sigin[1].vec;//signal delay time
		float[] in3  = sigin[2].vec;//signal feedback
		float[] out  = sigout[0].vec;//output
		float is;
		for(int i = 0;i < in.length;i++)
	    {
			_read_index = (int)(_write_index - in2[i] * 44.1);
			if(_read_index < 0)
			    _read_index = _read_index + _bufsize;
			is = in[i] + _last_sample;
			_delay_line[_write_index] = is;
			out[i] = _delay_line[_read_index]; 
			_last_sample = _delay_line[_read_index] * in3[i];
			_write_index++;
			if(_write_index >= _bufsize)
				_write_index = 0;
			if(_read_index >= _bufsize)
				_read_index = 0;
	    }
    }
    //MSPPerformable interface
    public void dspsetup(MSPSignal[] sigs_in, MSPSignal[] sigs_out)
    {
		dsp(sigs_in,sigs_out); 
    }

    public void perform(MSPSignal[] in,MSPSignal[] out)
    {
		if(in[1].connected && !in[2].connected)//signal rate delay time
		    p2(in,out);
		else if(in[1].connected && in[2].connected)//signal rate delay time and feedback
		    p3(in,out);
		else
		    p(in,out);
    }

}





