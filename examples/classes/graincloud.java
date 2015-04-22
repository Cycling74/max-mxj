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
import java.util.Vector;
import java.util.Random;

//simple graincloud implementation in Java. Could be optimized HEAVILY. 
//Ideally we would not create a new object for each grain or use some sort of
//object pooling.Also, we could get rid of the array copies and maintain the
//grain as indexes into the buffer.
//The main problem is all this object creation/copying triggers the GC which is bad
//for audio.When I get around to it I will take a stab at a version with minimal object
//creation and sample copying
//
//It would also be nice to add randomized pitch to each grain.
//
//This is just a simple, quick and dirty implementation
//-turtleman

public class graincloud extends MSPPerformer
{
	private int maxgrains = 512;

	private String bufname;
	private double sr 	  	= 0;
	private float density 	= 1; //in grains per second
	private float minlen = 2;	//min grain length in ms
	private float maxlen = 25; //max grain length in ms

	private grain[]  grains;
	private int active_grains = 0;
	private Random r;

	private static final String[] INLET_ASSIST = new String[]{
		"messages in"
	};

	private static final String[] OUTLET_ASSIST = new String[]{
		"signal out L",
		"signal out R",
	};
	public graincloud()
	{
		bail("(mxj~ graincloud) must provide buffer name as argument.");
	}

	public graincloud(String buf)
	{

		declareInlets(new int[]{SIGNAL});
		declareOutlets(new int[]{SIGNAL, SIGNAL});

		setInletAssist(INLET_ASSIST);
		setOutletAssist(OUTLET_ASSIST);
		
		grains  = new grain[maxgrains];
		bufname = buf;
		r = new Random();

		declareAttribute("minlen",null,"setminlen");
		declareAttribute("maxlen");
		declareAttribute("density",null,"setdensity");
		declareAttribute("maxgrains",null,"setmaxgrains");	
		declareAttribute("bufname");
	}

	private void setminlen(float ml)
	{
		if(ml < 0)
			ml = 0.001F;
	
		minlen = ml;	
	}

	private void setdensity(float d)
	{
		if(d > maxgrains)
			d = maxgrains;
		else if(d < 0)
			d = 0;	
		density = d;	
	}
	private void setmaxgrains(int max_grains)
	{
		maxgrains = max_grains;	
		active_grains = 0;	
		grains = new grain[maxgrains];
		if(density > maxgrains)
			density = maxgrains;	
	}

	public void dspsetup(MSPSignal[] ins, MSPSignal[] outs)
	{
		sr = outs[0].sr;	
	}

	public void perform(MSPSignal[] ins, MSPSignal[] outs)
	{

		int i,q,l;
		float[] in = ins[0].vec;		
		float[] outl = outs[0].vec;
		float[] outr = outs[1].vec;
		float p;
		
		p = (float)(density / sr);
		
		float s1,s2;
		grain g;

		for(i = 0; i < outl.length;i++)
		{
			if(r.nextFloat() <= p)
				_spawn_grain();

			s1 = s2 = 0;
			
			for(q = 0;q < active_grains;q++)
			{
				g = grains[q]; 			
				s1 += g.samps_l[g.cur_idx];
				s2 += g.samps_r[g.cur_idx];
				g.cur_idx++;
				if(g.cur_idx > g.length - 1)
				{
					if(q != active_grains - 1)
					{
						System.arraycopy(grains,q + 1, grains, q ,active_grains - 1);
						active_grains--;	
						q--;	
					}	
					else	
						active_grains--;	
				}
			}
				outl[i] = s1;
				outr[i] = s2;	
		}

	}
	
	private void _spawn_grain()
	{
		grain g = new grain(bufname,minlen,maxlen,r,sr);
		grains[active_grains] = g;	
		active_grains++;
	}

	class grain
	{

		float[] samps_l = null;
		float[] samps_r = null;
		int length;
		int cur_idx;
		
		public grain(String bufname,float min_len, float max_len,Random r, double sr)
		{

			if(MSPBuffer.getSize(bufname) == 0)
			{
				samps_l = new float[0];
				samps_r = new float[0];		
				return;
			}
			
			int num_channels    =  MSPBuffer.getChannels(bufname);
			double 	srms 		= sr / 1000;		
			int 	grainsize  	= (int)(((r.nextFloat() * (max_len - min_len)) + min_len) * srms);
			int		buflength  	= (int)(MSPBuffer.getFrames(bufname) / num_channels);
			int 	slocation  	= (int)((r.nextFloat() * buflength));

			int 	env_at_len 	= (int)(grainsize / 4); 
			float 	envstep 	= (float)1 / env_at_len;

			float 	amp 		= (float)r.nextFloat();
			float 	pan			= (float)r.nextFloat();	
			float   pan_l       = (float)Math.sqrt(pan);
			float   pan_r		= (float)Math.sqrt(1 - pan);

			if(slocation + grainsize >= buflength)
				slocation = slocation - (grainsize + 1);

						
			samps_l = MSPBuffer.peek(bufname,0,slocation,grainsize);
			if(num_channels > 1)
			{	
				samps_r = MSPBuffer.peek(bufname,1,slocation,grainsize);
			}	
			else
			{
 				samps_r = new float[samps_l.length];		 					
				System.arraycopy(samps_l,0,samps_r,0,samps_l.length);		
			}

			int i;
			float s = 0;	
			for(i = 0; i < env_at_len;i++)//attack
			{
				samps_l[i] = samps_l[i] * s * amp * pan_l;
				samps_r[i] = samps_r[i] * s * amp * pan_r;	
				s += envstep;
			}
			
			for(i = env_at_len;i < samps_l.length - env_at_len;i++)
			{
				samps_l[i] = samps_l[i] * amp * pan_l;
				samps_r[i] = samps_r[i] * amp * pan_r;
			}

			for(i = samps_l.length - env_at_len;i < samps_l.length;i++)//decay
			{	
				samps_l[i] = samps_l[i] * s * amp * pan_l;
				samps_r[i] = samps_r[i] * s * amp * pan_r;
				s -= envstep;	
			}

			length 	= samps_l.length;
			cur_idx = 0;
		}
	}
}	




























