package com.cycling74.msp;
import com.cycling74.max.MessageReceiver;
import javax.sound.sampled.*;
import java.io.FileNotFoundException;
import java.io.*;

/**
 * A utility class for loading audio data off of disk and into memory. The data is translated from the
 * native file format into the the floating point format used by msp.
 * @author Topher LaFata
 */
public class AudioFileBuffer
{
	/**
	* If an instance of MessageReceiver is passed into the constructor the FINISHED_READING message will be
	* sent to it when the requested file is successfully loaded into memory. File loading occurs asynchronously.	
	*/
	public static final int FINISHED_READING = 1;
	//file writing is not yet implemented
	//public static final int FINISHED_WRITING = 2;
	
    private File              _file;
    private AudioInputStream _ais;
    private AudioFormat      _aformat;
    private int              _num_channels;
    private long _framelength; //length of current file is frames
    private int _framesize;//size of each frame in bytes
    private int _sample_size_in_bits;
    private boolean _big_endian;
    private float            _sr;

	/**
	* buf contains the audio samples loaded off of disk deinterleaved by channel into a 2 dimensional
	* floating point array. The first dimension corresponds to the audio channel and the second dimension 
	* corresponds to the audio data itself. Thus a stereo audio file would have the samples for the left channel
	* at buf[0][0...framelength - 1] and the right channel at buf[1][0...framelength - 1]
	*/
    public float[][] buf;

    //this is affected by sample rate
    private MessageReceiver _client = null;
    private buf_filler _bft;
	
	/**
     * Constructor. 
	 * @param filename Absolute native path of the audio file to be loaded into memory.
     */
    public AudioFileBuffer(String filename)throws FileNotFoundException, IOException, UnsupportedAudioFileException
    {
		_client = null;
		open(filename);
    }

	/**
     * Constructor. 
	 * @param filename Absolute native path of the audio file to be loaded into memory.
	 * @param client instance of MessageReceiver which will be notified when file is finished being loaded into memory.
	 * This information may or may not be relevant since the member buffer,buf[][], containing the audio data will be valid and zero filled
	 * when the constructor returns. 
     */
    public AudioFileBuffer(String filename, MessageReceiver client)throws FileNotFoundException, IOException, UnsupportedAudioFileException
    {
		_client = client;
		open(filename);
    }
	/**
     * Load a different audio file into memory using this instance of AudioFileBuffer. Previous audio data is disgarded and
	 * buf[][] member variable reflects the number of channels and framesize of the new audio file..
	 * @param filename Absolute native path of the audio file to be loaded into memory.
     */    
    public void open(String filename)throws FileNotFoundException, IOException, UnsupportedAudioFileException
    {
		_file = new File(filename);
		if(!_file.exists())
		    throw new FileNotFoundException("Unable to find file "+filename);
		//kill the previous buf filler if it is already running
		kill_buf_filler();
		//This is so we have mark supported and get efficiency in our disk reads
		InputStream fileInputStream = new FileInputStream(_file);
		InputStream inputStream = new BufferedInputStream(fileInputStream);	
		try{
		    _ais = AudioSystem.getAudioInputStream(inputStream);
		}catch(IOException ioe)
		{
			throw ioe;
		}
		catch(UnsupportedAudioFileException uafe)
		{
			throw uafe;
		}	

		_aformat = _ais.getFormat();
		if(_aformat.getEncoding() != AudioFormat.Encoding.PCM_SIGNED)
		{
			throw new UnsupportedAudioFileException("AudioBuffer currently  only supports"+
								" PCM_SIGNED encodings");
		}
	
		_sr           = _aformat.getSampleRate();
		_num_channels = _aformat.getChannels();
		_framelength  = _ais.getFrameLength();
		_framesize    = _aformat.getFrameSize();
		_sample_size_in_bits = _aformat.getSampleSizeInBits();
		_big_endian   = _aformat.isBigEndian();

		buf          = new float[_num_channels][(int)_framelength];
		_bft = new buf_filler(this);
		_bft.start();

	}
  /**
  * Get the sample rate of the current audio file. It is important to note that
  * the current msp sampling rate is currently not considered when the audio file
  * is being decoded from disk. This means that an audio file saved with a different
  * sampling rate from the current msp sampling rate needs additional sample rate conversion
  * done on its audio data after it is loaded to play back at the expected speed.
  * This conversion is currently not done by default. 
  */
    public float getSampleRate()
    {
		return _sr;
    }
  /*
    public AudioFormat getAudioFormat()
    {
		return _aformat;
    }
*/
	/**
	* Get the sample size in bits of the current audio file. For example, 8,16,24.
	*/
    public int getSampleSizeInBits()
    {
		return _sample_size_in_bits;
    }
	/**
	* Was the current audio file big endian format.
	*/
    public boolean isBigEndian()
    {
		return _big_endian;
    }
    /**
	* Get the number of sample frames in the audio file.A frame consists of
	* sample data for all channels at a particular instant in time.Thus a mono
	* audio file which has 1000 samples will have a frame length of 1000 with
	* each frame containing one sample. A stereo audio file with 2000 samples would
	* have a frame length of 1000 with each frame containing 2 samples, one for each
	* the left and right channels.
	*/
    public long getFrameLength()
    {
		return _framelength;
    }
	/**
	* Get the number of channels of the current audio file.
	*/
    public int getChannels()
    {
		return _num_channels;
    }
    
    /**
	*Get the length of the current audio file in milliseconds.
	* This is equivalent to:
	*<pre>
	* frame length / (sample rate / 1000)
	*</pre>
	*/
    public float getLengthMs()
    {
		return (float)(_framelength / (getSampleRate() / 1000));
    }
    
    
   private void fill_buf()
    {		
		byte[] tmp = new byte[2048];
		int bytesread = 0;
		short ss = 0;
		int si = 0;
		int wh = 0;

	try{
	    while((bytesread = _ais.read(tmp,0,tmp.length)) > 0)
		{
		    for(int i = 0; i < bytesread;i+= _framesize)
			{
			    //if it is not bigendian make it so now...
			    //this is happening per frame..maybe not the best approach
			    if(!_big_endian && _sample_size_in_bits > 8)
				{
				    byte t1 = 0;
				    switch(_sample_size_in_bits)
					{
					case 16:
					    for(int ii = i; ii < i + _framesize;ii += 2)
						{
						    t1 = tmp[ii];
						    tmp[ii] = tmp[ii+1];
						    tmp[ii +1] = t1;
						}
					    break;
					case 24:
					    for(int ii = i; ii < i + _framesize;ii += 3)
						{
						    t1 = tmp[ii];
						    tmp[ii] = tmp[ii+3];
						    tmp[ii +3] = t1;
						}
					    break;
					default:
					}
				}
			    //deinterleave and convert to float
			    for(int c = 0;c < _num_channels;c++)
				{
				    int ch_offset = (c * _num_channels);
				    switch (_sample_size_in_bits)
					{
					case 8:
					    ss = (short)(tmp[i+c] & 0xff);
					    buf[c][wh] = (float)ss/16384;
					    break;
					case 16:
					    ss = (short)(((tmp[i+ch_offset] & 0xff) << 8) | (tmp[i+ch_offset+1] & 0xff));
					    buf[c][wh] = (float)ss/Short.MAX_VALUE;
					    break;
					case 24:
					    si = (int)( ((tmp[i+ch_offset] & 0xff) << 16) | ((tmp[i+ch_offset+1] & 0xff) << 8) | (tmp[i+ch_offset+2] & 0xff));
					    buf[c][wh] = (float)si/8388608;
			 		    break;
					default:
					}

				}
			    wh++;
			}
		}
	    _ais.close();
		if(_client != null)
		{
			_client.messageReceived(this,FINISHED_READING,null);
		}
	}catch(Exception e)
	    {
		e.printStackTrace();
	    }

    }

	private void kill_buf_filler()
	{
	    if(_bft != null)
		{
		    try{
				_bft.join();	
		    }catch(InterruptedException e)
			{
			    e.printStackTrace();
			}		
		}		
	}


    class buf_filler extends Thread
    {
		AudioFileBuffer _p = null;
	
		public buf_filler(AudioFileBuffer p)
		{
			_p = p;
		}
	
		public void run()
		{
			_p.fill_buf();
		}
	
    }    

 
}

