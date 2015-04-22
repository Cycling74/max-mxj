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

public class noiseperformer extends MSPPerformer
{

    long _last_val;

    private static final long jflone = 0x3f800000;
    private static final long jflmsk = 0x007fffff;

	private static final String[] INLET_ASSIST = new String[]{
		"messages in"
	};

	private static final String[] OUTLET_ASSIST = new String[]{
		"white noise out (sig)"
	};

    public noiseperformer()
    {
		declareInlets(new int[]{DataTypes.ALL});
		declareOutlets(new int[]{SIGNAL});
		
		setInletAssist(INLET_ASSIST);
		setOutletAssist(OUTLET_ASSIST);

		_last_val = System.currentTimeMillis();
    }

    public void perform(MSPSignal[] in, MSPSignal[] out)
    {
		long tmp;
		long idum,itemp;
		int i;
		idum = _last_val;
	
		float[] o = out[0].vec;
	
		for(i = 0; i < o.length;i++)
	    {
			idum = 1664525L * idum + 1013904223L;
			itemp = jflone  | (jflmsk & idum );
			o[i] = ((Float.intBitsToFloat((int)itemp)) *
				(float)2.0 ) - (float)3.0;
		}
		_last_val = idum;
    }
	
}









