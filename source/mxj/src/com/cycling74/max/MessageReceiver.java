package com.cycling74.max;

/**
 * MessageReceiver provides a common interface for Objects to be notified of
 * potentially interesting events asynchronously. For instance, an object implementing
 * the MessageRecevier interface can be notified when an instance of com.cycing74.msp.AudioFileBuffer
 * has completed loading audio file data off of disk.
 **/
 
 public interface MessageReceiver
{
	/**
	 * This method will be called when an event occurs.
	 *  @param src the object from which the message originated
	 *  @param message_id the integer id of the message
	 *  @param data any data accompanying the message. This may be null if there is no data
	 *   accompanying the message.
	 */
    public void messageReceived(Object src,int message_id,Object data);
}
