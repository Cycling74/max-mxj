package com.cycling74.msp;

/**
 * Provides static methods that get and set buffer data.
 * 
 * created on 9-April-2004
 * @author Ben Nevile
 */
public class MSPBuffer
{
	//private String _name;
	
	//at this point an instance of MSPBuffer cannot be created.
	//instead we have static methods that can be used to access
	//buffers that already exist (ie, they've been made in the traditional
	//way with the buffer~ object.)
	private MSPBuffer() {	}
	

	//get an exisiting buffer, returns null if it doesn't exist
//	public static MSPBuffer getBuffer(String name)
//	{
//		long ptr = _get_msp_buffer_ptr(name);
//		if (ptr == 0)
//			return null; //no buffer named name
//		MSPBuffer b =  new MSPBuffer(ptr);
//		b._name = name;
//		return b;
//	}
	
	
	/**
	 * Gets the number of channels.
	 * @param name the name of the buffer
	 * @return the number of channels
	 */
	public static native int getChannels(String name);
	/**
	 * Sets the length of a buffer in milliseconds, and the number of channels.
	 * @param name the name of the buffer
	 * @param numchannels the number of channels for the buffer to have
	 * @param millis the length, in milliseconds
	 */
	public static native void setLength(String name, int numchannels, double millis);	
	/**
	 * Gets the length of a buffer in milliseconds.
	 * @param name the name of the buffer
	 * @return the length in milliseconds
	 */
	public static native double getLength(String name);
	/**
	 * Gets the number of frames (samples) in a buffer.
	 * @param name the name of the buffer
	 * @return the number of frames in the buffer
	 */
	public static native long getFrames(String name);
	/**
	 * Sets the number of frames (samples) in a buffer, and the number of channels.
	 * Note that the execution of this method is always deferred to the main thread.
	 * @param name the name of the buffer
	 * @param numchannels the number of channels for the buffer to have
	 * @param size the length of the buffer, in frames
	 */
	public static native void setFrames(String name, int numchannels, long size);
	/**
	 * Gets the size (frames*channels) of a buffer.
	 * @param name the name of the buffer
	 * @return the size (frames*channels)
	 */
	public static native long getSize(String name);
	/**
	 * Sets the size (frames*channels) of a buffer, and the number of channels.
	 * If the size is not evenly divided by the number of channels it is rounded down. 
	 * Note that the execution of this method is always deferred to the main thread.
	 * @param name the name of the buffer
	 * @param numchannels the number of channels for the buffer to have
	 * @param size the new size of the buffer, in samples, to be distributed 
	 * amongst all the channels
	 */
	public static native void setSize(String name, int numchannels, long size);
	/**
	 * Get a single value from a buffer.  Indexing starts at 0.
	 * @param name the name of the buffer
	 * @param channel the channel of interest
	 * @param index the index within the channel to get
	 * @return the value of the sample at the requested channel and index
	 */
	public static native float peek(String name, int channel, long index);
	/**
	 * Get a range of values from a buffer.  Indexing starts at 0.
	 * @param name the name of the buffer
	 * @param channel the channel of interest
	 * @param start the first index of interest
	 * @param length the number of samples to return
	 * @return the values of the samples from the requested channel and range
	 */
	public static native float[] peek(String name, int channel, long start, long length);
	/**
	 * Get an entire channel from a buffer.
	 * @param name the name of the buffer
	 * @param channel the channel of interest
	 * @return the values of the samples from the requested channel
	 */
	public static native float[] peek(String name, int channel);
	/**
	 * Get the data from an entire buffer with the channels interleaved.
	 * @param name the name of the buffer
	 * @return the buffer data with interleaved channels 
	 */
	public static native float[] peek(String name);
	/**
	 * Set one data point in a buffer.
	 * @param name the name of the buffer
	 * @param channel the channel of interest
	 * @param index the index to set
	 * @param val the new value at the given channel and index
	 */
	public static native void poke(String name, int channel, long index, float val);
	/**
	 * Set a range of data in a buffer.
	 * @param name the name of the buffer
	 * @param channel the channel of interest
	 * @param start the first index in the range
	 * @param val the data to write into the buffer
	 */
	public static native void poke(String name, int channel, long start, float[] val);
	/**
	 * Set a channels worth of data in a buffer.
	 * @param name the name of the buffer
	 * @param channel the channel of interest
	 * @param val the data write into the buffer in the channel of interest
	 */
	public static native void poke(String name, int channel, float[] val); 
	/**
	 * Set all the data in a buffer in interleaved format.
	 * @param name the name of the buffer
	 * @param val the data to write into the buffer, with the channels interleaved.
	 */
	public static native void poke(String name, float[] val);
	

}