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

public class shape1 extends MSPPerformer
{
    float _k;

	public shape1()
	{
		this(0);
	}
    public shape1(float f)
    {
		declareInlets(new int[]{SIGNAL});
		declareOutlets(new int[]{SIGNAL});
		_k =  2 * f / 1 - f;
    }

    public void inlet(float f)
    {
		_k =  2 * f / 1 - f;
    }
    
    public void perform(MSPSignal[] ins, MSPSignal[] outs)
    {
		 float[] in  = ins[0].vec;
		 float[] out = outs[0].vec;
		 float si = 0;
		 float sabs = 0;

		for(int i = 0; i < in.length;i++)
		{
			si = in[i];
		  	if(si < 0 )
				sabs = -si;
		 	else
				sabs = si;

	  		out[i] = ((1 + _k) * si/ (1 +_k * sabs));
		}
    }
    
	
}



