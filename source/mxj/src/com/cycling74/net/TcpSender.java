package com.cycling74.net;
import com.cycling74.max.*;
import java.net.*;
import java.io.*;
/**
 * TCP Sender.
 * 
 * created on 24-May-2004
 * @author bbn
 */
public class TcpSender {
    private InetAddress iAddress = null;
    private String address = null;
    private int port = 0;
    private boolean goodParams = false;
    private String debugString = "TcpSender";
    private Callback successCallback = null;
    private Callback failureCallback = null;
    private int activePackets = 0;
    
    private class Sender extends Thread {
    	private String s;
    	Sender(String s) {
    		activePackets++;
    		this.s = s;
    	}    	
		public void run() {			
		  try {
				Socket sendSocket = new Socket(iAddress, port);
				PrintWriter writer = new PrintWriter(sendSocket.getOutputStream());
				writer.println(s);
				writer.close();
				activePackets--;
				if (successCallback!=null) {
					successCallback.setArgs(new Object[] {Atom.parse(s)});
					successCallback.execute();
				}
			} catch (SocketException se) {
				activePackets--;
				if (failureCallback!=null) {
					failureCallback.setArgs(new Object[] {Atom.parse(s)});
					failureCallback.execute();
				} else {
					MaxSystem.error(debugString+": socket exception: "+se);
					MaxSystem.post("not sent: "+s);
				}
			} catch (IOException ie) {
				activePackets--;
				if (failureCallback!=null) {
					failureCallback.setArgs(new Object[] {Atom.parse(s)});
					failureCallback.execute();
				} else {
					MaxSystem.error(debugString+": io exception: "+ie);
					MaxSystem.post("not sent: "+s);
				}
			} 
		}
	}

    /**
     * Creates a TcpSender with specified address and port.
     * @param address the address to send to
     * @param port the port to send to
     */
    public TcpSender(String address, int port) {
    	setAddress(address);
    	setPort(port);
	}
    
    /**
     * Creates a TcpSender.  Address and port must be set 
     * before anything will work.
     */
    public TcpSender() {
    }

    private void initSendSocket(String addrarg, int portarg) {
    	if ((addrarg!=null)&&(portarg!=0)) {
			try {
				goodParams = false;
			    iAddress = InetAddress.getByName(addrarg);
			    port = portarg;
			    goodParams = true;
			} catch (UnknownHostException uhe) {
			    MaxSystem.error(debugString+": unknown host: "+addrarg);
			}
    	}
    }	
    
    /**
     * Sets the success callback.  The method will be called in the event
     * of a successful data transmission.  This method must take an array of Atoms
     * as its only argument.
     * @param toCallIn the object that contains the method
     * @param methodName the name of the method
     */
    public void setSuccessCallback(Object toCallIn, String methodName) {
    	if ((toCallIn != null)&&(methodName != null)) {
			successCallback = new Callback(
						toCallIn, 
						methodName, 
						new Object[] {Atom.emptyArray}
						);	
    	}
    }
    /**
     * Sets the failure callback.  The method will be called in the event of a 
     * failed data transmission.  This method must take an array of Atoms
     * as its only argument.
     * @param toCallIn the object that contains the method
     * @param methodName the name of the method
     */
    public void setFailureCallback(Object toCallIn, String methodName) {
    	if ((toCallIn != null)&&(methodName != null)) {
			failureCallback = new Callback(
						toCallIn, 
						methodName, 
						new Object[] {Atom.emptyArray}
						);	
    	}
    }
    
    /**
     * Set the IP address.
     * @param address the IP address to send to
     */
    public void setAddress(String address) {
		if (!address.equals(this.address)) {
			this.address = address;
			initSendSocket(address, port);
		}
    }
    
    /**
     * Gets the IP address.
     * @return the IP address that data is being sent over
     */
    public String getAddress() {
    	return address;
    }
    
    /**
     * Set the port.
     * @param port the port to send to
     */
    public void setPort(int port) {
		if (port != this.port) {
			this.port = port;
			initSendSocket(address, port);
		}
    }
    
    /**
     * Sets the debug String.
     * @param debugString the String to be displayed as part of the error message
     * in the event of a networking error.
     */
    public void setDebugString(String debugString) {
    	this.debugString = debugString;
    }
    /**
     * Get the port.
     * @return the port being sent to
     */
    public int getPort() {
    	return port;
    }
    
    /**
     * Get the number of active packets.  A packet is active if has been
     * sent but it is not yet known if it was sent successfully.  
     * @return the number of active packets
     */
    public int getActivePackets() {
    	return activePackets;
    }

	/**
	 * Send an integer.
	 * @param i the integer to send
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
	 * Send a message.
	 * @param msg the message to send
	 * @param a arguments to the message
	 */
	public void send(String msg, Atom[] a) {
		send(msg + " " +  Atom.toOneString(a));
	}
	/**
	 * Send a list.
	 * @param a array of Atoms to send.
	 */
	public void send(Atom[] a) {
		send(Atom.toOneString(a));
	}
	
  private void send(String s) {
		if (goodParams) {
			Sender sd = new Sender(s);
			sd.start();
		}
  }
   
   /**
    * Should be called when the TcpSender is no longer needed.
    */
    public void close() {
   		successCallback = null;
   		failureCallback = null;
   }
}

