package com.cycling74.net;
import com.cycling74.max.*;
import java.net.*;
import java.io.IOException;
/**
 * Send to a multicast group.
 * 
 * created on 17-May-2004
 * @author Ben Nevile
 */
public class MultiSender {
	private InetAddress group = null;
	private String groupName;
	private String debugString;
	private int port;
	private int timeToLive;
    private MulticastSocket sendSocket = null;
    private boolean readyToSend = false;
    private boolean validGroup = false;
    private boolean validPort = false;
       
    /**
     * Create a new MultiSender with specified group name, port, 
     * debug string, and time to live.  
     * 
     * @param groupName group to send to
     * @param port port to send on
     * @param debugString part of the output to the Max window in the event of a 
     * networking error
     * @param timeToLive number of hops the transmission will stay alive
     */
    public MultiSender(String groupName, int port, String debugString, byte timeToLive) {
    	setDebugString(debugString);
    	setTimeToLive(timeToLive);
    	if (port > 0) 
    		setPort(port);
    	if (groupName!=null)
    		setGroup(groupName);		
		init();	
    }
    
    /**
     * Create a new MultiSender with specified group, port, and time to live.
     * @param groupName group to send to
     * @param port port to send on
     * @param timeToLive number of hops the transmission will stay alive
     */
    public MultiSender(String groupName, int port, byte timeToLive) {
    	this(groupName, port, "MultiSender", timeToLive);
    }
    
    /**
     * Create a new MultiSender with specified group and port.
     * Time to live is set to a default of 1.
     * @param groupName group to send to
     * @param port port to send on
     */
    public MultiSender(String groupName, int port) {
    	this(groupName, port, (byte)1);
    }
    
    /**
     * Create a new MultiSender.  group and port must be specified
     * with setGroup and setPort before anything can happen.
     */
    public MultiSender() {
    	this(null, 0);    	
    }
    
    /**
     * Set the port.
     * @param p port to send over.
     */
    public void setPort(int p) {
    	port = p;
    	validPort = true;
    }
    
    /**
     * Get the port.
     * @return the port data is being sent over.
     */
    public int getPort() {
    	return port;
    }
    
    /**
     * Set the group.
     * @param s the group to send to
     */
    public void setGroup(String s) {
    	groupName = s;
    	(new Thread() {
    		public void run() {
	    		try {
	    			validGroup = false;
	    			group = InetAddress.getByName(groupName);
	    			validGroup = true;
	    		} catch (UnknownHostException uhe) {
					MaxSystem.error(debugString + ": unknown group. "+uhe);
				}
    		}
    	}).start();
    }
    
    /**
     * Get the group.
     * @return the group data is being sent to
     */
    public String getGroup() {
    	return groupName;
    }
    
    /**
     * Sets time to live.
     * @param i the transmission will remain alive for this number of hops.
     */
    public void setTimeToLive(int i) {
    	timeToLive = i;
    }
    
    /**
     * Gets time to live.
     * @return the number of hops that a transmission will stay alive.
     */
    public byte getTimeToLive() {
    	return (byte)timeToLive;
    }
    
    /**
     * Sets the debug string.
     * @param s the debug string
     */
    public void setDebugString(String s) {
    	debugString = s;
    }
        
    private void init() {
    	(new Thread() {
	    	public void run() {
	    		try {
	    			readyToSend = false;
					sendSocket = new MulticastSocket();
					readyToSend = true;
				} catch (IOException ioe) {
					MaxSystem.error(debugString + ": io error." + ioe);
				}
	    	}
    	}).start();
    }

	/**
	 * Send a string.
	 * @param msg the String to send
	 */
	private void send(String msg) {
		if (readyToSend&&validGroup&&validPort) {
			try {
				byte b[] = msg.getBytes("UTF-8");
				DatagramPacket packet = 
					new DatagramPacket(b, b.length, group, port);
					int oldTtl = sendSocket.getTimeToLive();
					sendSocket.setTimeToLive(timeToLive);
					sendSocket.send(packet);
					sendSocket.setTimeToLive(oldTtl);
			} catch (IOException ioe) {
				MaxSystem.error(debugString + ": io error." + ioe);
			} 
		} else {
			MaxSystem.error(debugString + "not ready to send.");
		}
	}
	
	/**
	 * Send an integer.
	 * @param i the int to send
	 */
	public void send(int i) {
		send(new Integer(i).toString());
	}
	/**
	 * Send a float.
	 * @param f the float to send
	 */
	public void send(float f) {
		send(new Float(f).toString());
	}
	/**
	 * Send an Atom array.
	 * @param a the Atom array to send.
	 */
	public void send(Atom[] a) {
		send(Atom.toOneString(a));
	}
	/**
	 * Send a message.
	 * @param msg the message to send
	 * @param a the arguments to send with the message
	 */
	public void send(String msg, Atom[] a) {
		send(msg + " " + Atom.toOneString(a));
	}
}
