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

public class audiofilebuf extends MSPPerformer implements MessageReceiver
{
	
	private AudioFileBuffer _afb = null;
	private int _rh;

	private static final String[] INLET_ASSIST = new String[]{
		"messages in"
	};
	private static final String[] OUTLET_ASSIST = new String[]{
		"output L","output R"
	};
	public audiofilebuf()
	{
		bail("must provide audio file name.");
	}

	public void open(String name)
	{
		String fname = MaxSystem.locateFile(name);
		if(fname == null)
		{
			post("unable to find name");			
			return;
		}

		try{

			_afb.open(fname);

		}catch(Exception e)
		{
			bail(e.getMessage());
			e.printStackTrace();
		}

		System.out.println("frame length: "+ _afb.getFrameLength());
		System.out.println("ss in bits  : "+ _afb.getSampleSizeInBits());
		System.out.println("num channels: "+ _afb.getChannels());
		System.out.println("big endian  : "+ _afb.isBigEndian());
		System.out.println("sample rate  : "+ _afb.getSampleRate());
		_rh = 0;
	}
	public audiofilebuf(String name)
	{
		declareInlets(new int[]{SIGNAL});
		declareOutlets(new int[]{SIGNAL,SIGNAL});

		setInletAssist(INLET_ASSIST);
		setOutletAssist(OUTLET_ASSIST);
		String fname = MaxSystem.locateFile(name);

		if(fname == null)
			bail("Unable to find "+name);	

		try{
			_afb = new AudioFileBuffer(fname,this);
		}catch(Exception e)
		{
			bail(e.getMessage());
			e.printStackTrace();
		}

		System.out.println("frame length: "+ _afb.getFrameLength());
		System.out.println("ss in bits  : "+ _afb.getSampleSizeInBits());
		System.out.println("num channels: "+ _afb.getChannels());
		System.out.println("big endian  : "+ _afb.isBigEndian());
		System.out.println("sample rate  : "+ _afb.getSampleRate());
		_rh = 0;

	}
    

	public void perform(MSPSignal[] ins, MSPSignal[] outs)
	{
		int i;
		float[] out1 = outs[0].vec;
		float[] out2 = outs[1].vec;
		float s;
		//mono	
		if(_afb.getChannels() == 1)
		{
			for(i = 0; i < out1.length;i++)
			{
				if(_rh > _afb.buf[0].length - 1)
					_rh = 0;

				s  = _afb.buf[0][_rh++];  	

				out1[i] = s;
				out2[i] = s;	
			}
		}
		else
		{
		//stereo
			for(i = 0; i < out1.length;i++)
			{
				if(_rh > _afb.buf[0].length - 1)
					_rh = 0;
	
				out1[i] = _afb.buf[0][_rh];  	
				out2[i] = _afb.buf[1][_rh];  	
				_rh++;	
			}
		}
		
	}

	//MessageReceiver interface
	public void messageReceived(Object src,int message_id,Object data)
	{
		switch (message_id)
		{
			case AudioFileBuffer.FINISHED_READING:
				post("file is done loading");
		}
	}

}




