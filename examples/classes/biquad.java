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

public class biquad extends MSPObject implements MSPPerformable
{
	private static final double LN_2_2 = 0.34657359028;
	private static final double M_LN2 = 0.69314718055994530942;


	public static final int PEAK		= 0;
	public static final int LOWSHELF 	= 1;
	public static final int HISHELF 	= 2;
	public static final int LOWPASS 	= 3;
	public static final int HIPASS 		= 4;
	public static final int BANDPASS 	= 5;

	//filter coeffs
	private double a1;
	private double a2;
	private double b0;
	private double b1;
	private double b2;
	private double x1;
	private double x2;
	private double y1;
	private double y2;
	
	//sampling rate
	private double _sr;

	//attributes
	public double cutoff;
	public double gain;
	public double bw;//bw or slope for shelving
	public int    type;

	//perform Methods
	private Method _p1   = null;
	private Method _p2   = null;
	

	//assist messages
	private static final String[] INLET_ASSIST = 
		new String[]{"(sig) input","(float/sig)cutoff","(float/sig)bw or slope","(float/sig)gain db"};
	private static final String[] OUTLET_ASSIST = 
		new String[]{"(sig) output"};

	//constructors
	public biquad()
	{
		bail("(mxj biquad)missing args.usage: [mxj biquad type cutoff q/slope gain]");  
	}

	public biquad(String type, double cutoff)
	{
		this(typeToInt(type),cutoff,0.3f,0f);	
	}

	public biquad(String type, double cutoff, double bw)
	{
		this(typeToInt(type),cutoff,bw,0f);	
	}	
	
	public biquad(String type, double cutoff, double bw, double gain)
	{
		this(typeToInt(type),cutoff,bw,gain);	
	}


	public biquad(int type,double cutoff,double bw, double gain )
	{	
		declareInlets(new int[]{SIGNAL,SIGNAL,SIGNAL,SIGNAL});
		declareOutlets(new int[]{SIGNAL});
	
		setInletAssist(INLET_ASSIST);
		setOutletAssist(OUTLET_ASSIST);
		
		declareAttribute("type");
		declareAttribute("gain");
		declareAttribute("bw");
		declareAttribute("cutoff");

		if(type < 0 || type > 5)
		{
			type = 0;	
			post("(mxj biquad) unknown type "+type+"defaulting to PEAK");	
		}	

		this.type = type;
		this.gain 	= gain;
		this.bw 	= bw;
		this.cutoff = cutoff;

		_p1 = getPerformMethod("p1");
		_p2 = getPerformMethod("p2");	
		updateCoeffs();
		reset();
	}

	public void inlet(float f)
	{

		switch(getInlet())
		{
			case 1://cutoff
				cutoff = (double)f;
				break;
			case 2://bw/slope
				bw = (double)f;
				break;
			case 3://gain
				gain = (double)f;
				break;
			default:
				return;
		}
		updateCoeffs();
	}

	public void updateCoeffs()
	{
		switch(type)
		{
			case PEAK:
				calc_parametric(cutoff,gain,bw,_sr);	
				break;
			case LOWSHELF:
				calc_lowshelf(cutoff,gain,bw,_sr);
				break;
			case HISHELF:
				calc_hishelf(cutoff,gain,bw,_sr);
				break;
			case LOWPASS:
				calc_lowpass(cutoff,gain,bw,_sr);
				break;
			case HIPASS:
				calc_hipass(cutoff,gain,bw,_sr);
				break;
			case BANDPASS:
				calc_bandpass(cutoff,gain,bw,_sr);
				break;
		}
	}
	
	public Method dsp(MSPSignal[] ins, MSPSignal[] outs)
	{
		_sr = ins[0].sr;
		updateCoeffs();	
	
		if(ins[1].connected || ins[2].connected || ins[3].connected)			
			return _p2;//if singal input for cf,bw or gain use signal perform routine
		else
			return _p1;//use normal perform routine
	}

	public void p1(MSPSignal[] ins, MSPSignal[] outs)
	{		
		int i;
		float[] in  = ins[0].vec;
		float[] out = outs[0].vec;
		double y;
		for(i = 0; i < in.length;i++)
		{		
			y = b0 * in[i] + b1 * x1 + b2 * x2+ a1 * y1 + a2 * y2;		
			x2 = x1;
			x1 = in[i];
			y2 = y1;
			y1 = y;
			out[i] = (float)y;	
		}
	}

	public void p2(MSPSignal[] ins, MSPSignal[] outs)
	{		
		int i;
		float[] in 		= ins[0].vec;
		float[] cf		= ins[1].vec;
		float[] bwith 	= ins[2].vec;
		float[] gain  	= ins[3].vec;
	
		float[] out = outs[0].vec;

		double y;
		for(i = 0; i < in.length;i++)
		{	
			this.cutoff = cf[i];
			this.bw     = bwith[i];
			this.gain   = gain[i];
			updateCoeffs();
	
			y = b0 * in[i] + b1 * x1 + b2 * x2+ a1 * y1 + a2 * y2;		
			x2 = x1;
			x1 = in[i];
			y2 = y1;
			y1 = y;
			out[i] = (float)y;	
		}
	}


	//messages

	public void type(String t)
	{
		type = typeToInt(t);
		updateCoeffs();
		reset();
	}

	public void reset()
	{
		x1 = x2 = y1 = y2 = 0;		
	}


	//MSPPerformable Interface
	public void perform(MSPSignal[] ins, MSPSignal[] outs)
	{
		p1(ins,outs);
	}

	public void dspsetup(MSPSignal[] ins, MSPSignal[] outs)
	{	
		_sr = ins[0].sr;
		updateCoeffs();
	}
	//End MSPPerformable Interface


//coeff calculations
	private void calc_parametric(double fc, double gain, double bw, double fs)
	{
		double w = 2.0 * Math.PI * limit(fc, 1.0, fs/2.0) / fs;
		double cw = Math.cos(w);
		double sw = Math.sin(w);
		double J = Math.pow(10.0, gain * 0.025);
		double g = sw * sinh(LN_2_2 * limit(bw, 0.0001, 4.0) * w / sw);
		double a0r = 1.0f / (1.0f + (g / J));

		b0 = (1.0 + (g * J)) * a0r;
		b1 = (-2.0 * cw) * a0r;
		b2 = (1.0 - (g * J)) * a0r;
		a1 = -b1;
		a2 = ((g / J) - 1.0) * a0r;
	}

	private void calc_lowshelf(double fc, double gain, double slope, double fs)
	{
		double w = 2.0 * Math.PI * limit(fc, 1.0, fs/2.0) / fs;
		double cw = Math.cos(w);
		double sw = Math.sin(w);
		double A = Math.pow(10.0, gain * 0.025);
		double b = Math.sqrt(((1.0 + A * A) / limit(slope, 0.0001, 1.0)) 
					- ((A - 1.0) * (A - 1.0)));
		double apc = cw * (A + 1.0);
		double amc = cw * (A - 1.0);
		double bs = b * sw;
		double a0r = 1.0 / (A + 1.0 + amc + bs);

		b0 = a0r * A * (A + 1.0f - amc + bs);
		b1 = a0r * 2.0 * A * (A - 1.0f - apc);
		b2 = a0r * A * (A + 1.0 - amc - bs);
		a1 = a0r * 2.0 * (A - 1.0 + apc);
		a2 = a0r * (-A - 1.0 - amc + bs);
	}

	private void calc_hishelf(double fc, double gain, double slope, double fs)
	{
		double w = 2.0 * Math.PI * limit(fc, 1.0, fs/2.0) / fs;
		double cw = Math.cos(w);
		double sw = Math.sin(w);
		double A = Math.pow(10.0, gain * 0.025);
		double b = Math.sqrt(((1.0 + A * A) / limit(slope, 0.0001, 1.0)) 
					- ((A -1.0) * (A - 1.0)));
		double apc = cw * (A + 1.0);
		double amc = cw * (A - 1.0);
		double bs  = b * sw;
		double a0r = 1.0 / (A + 1.0f - amc + bs);

		b0 = a0r * A * (A + 1.0 + amc + bs);
		b1 = a0r * -2.0 * A * (A - 1.0f + apc);
		b2 = a0r * A * (A + 1.0 + amc - bs);
		a1 = a0r * -2.0 * (A - 1.0f - apc);
		a2 = a0r * (-A - 1.0 + amc + bs);
	
	}

	private void calc_lowpass(double fc, double gain, double bw, double fs)
	{
		double omega = 2.0 * Math.PI * fc/fs;
		double sn = Math.sin(omega);
		double cs = Math.cos(omega);
		double alpha = sn * sinh(M_LN2 / 2.0 * bw * omega / sn);
        double a0r = 1.0 / (1.0 + alpha);

        b0 = a0r * (1.0 - cs) * 0.5;
		b1 = a0r * (1.0 - cs);
        b2 = a0r * (1.0 - cs) * 0.5;
        a1 = a0r * (2.0 * cs);
        a2 = a0r * (alpha - 1.0);
	}

	private void calc_hipass(double fc, double gain, double bw, double fs)
	{
		double omega = 2.0 * Math.PI * fc/fs;
		double sn = Math.sin(omega);
		double cs = Math.cos(omega);
		double alpha = sn * sinh(M_LN2 / 2.0 * bw * omega / sn);
        double a0r 	= 1.0 / (1.0 + alpha);

        b0 = a0r * (1.0 + cs) * 0.5;
        b1 = a0r * -(1.0 + cs);
        b2 = a0r * (1.0 + cs) * 0.5;
        a1 = a0r * (2.0 * cs);
        a2 = a0r * (alpha - 1.0);

	}

	private void calc_bandpass(double fc, double gain, double bw, double fs)
	{
		double omega = 2.0 * Math.PI * fc/fs;
		double sn = Math.sin(omega);
		double cs = Math.cos(omega);
		double alpha = sn * sinh(M_LN2 / 2.0 * bw * omega / sn);
        double a0r = 1.0 / (1.0 + alpha);

        b0 = a0r * alpha;
        b1 = 0.0;
        b2 = a0r * -alpha;
        a1 = a0r * (2.0 * cs);
        a2 = a0r * (alpha - 1.0);

	}


//Util functions
	
	private static int typeToInt(String type)
	{	
		int itype;

		String t = type.toUpperCase();
		if(t.equals("PEAK") ||
			t.equals("EQ"))	
				itype = PEAK;
		else if(t.equals("LOWSHELF"))
				itype = LOWSHELF;
		else if(t.equals("HISHELF"))
				itype = HISHELF;
		else if(t.equals("LOWPASS"))
				itype = LOWPASS;
		else if(t.equals("HIPASS"))
				itype = HIPASS;
		else if(t.equals("BANDPASS"))
				itype = BANDPASS;
		else
		{
			post("(mxj biquad) unknown type: "+type+". Defaulting to PEAK.");
			itype = PEAK;
		} 
		return itype;	
	}

	private static double limit(double v,double l,double u) 
	{
		if(v < l)
			return l;
		else if(v > u)
			return u;
		else
			return v;
	}
   
	private static double sinh(double theta) 
	{
        return (Math.exp(theta)-Math.exp(-theta))/2;
    }

	private static float scale(float in, float from1, float from2, float to1, float to2)
	{

		return (in - from1) * (to2-to1)/(from2 - from1) + to1;
	}


}












