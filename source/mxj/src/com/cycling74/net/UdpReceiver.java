// created by bbn on May 23, 2004
package com.cycling74.net;

import com.cycling74.max.*;
import java.net.*;
import java.io.IOException;

/**
 * UDP receiver.
 * An internal Callback is maintained from a specified 
 * method name and object.  The specified method must 
 * take an array of Atoms as its only argument.  When
 * data is received, it is translated to an array of
 * Atoms and passed to the specified method.  When a
 * UdpReceiver is no longer needed its close() method 
 * should be called.
 *
 * @author Ben Nevile
 */
public class UdpReceiver implements Runnable {
	private static Class ATOM_ARRAY_CLASS = (new Atom[] {}).getClass();
    private int port;
    private DatagramSocket recvSocket;
    private Thread listener = null;
    private String debugString = "UdpReceiver";
	private Callback callback;
	
    /**
     * Creates a UdpReceiver with specified port.
     * @param port the port to listen to
     */
    public UdpReceiver(int port) {
		this.port = port;
		setActive(true);
    }
    
    /**
     * Creates a UdpReceiver.  Port must be specified before it will
     * receive anything.
     */
    public UdpReceiver() {
    }
    
    /**
     * Set the port.
     * @param port the port to listen to.
     */
    public void setPort(int port) {
		if (port != this.port) {
			this.port = port;
			resetListener();
		}
    }
    
    /**
     * Get the port.
     * @return the port being listened to
     */
    public int getPort() {
    		return port;
    }
    
    private void resetListener() {
    	(new Thread() {
    		public void run() {
    			setListener(false);
    			setListener(true);
    		}
    	}).start();
    }
    
    /**
     * Sets the debug string.
     * @param s the String to be output as part of the 
     */
    public void setDebugString(String s) {
    		debugString = s;
    }

    /**
     * Turns the UdpReceiver on or off.
     * @param b true turns it on
     */
    public void setActive(boolean b) {
    		if (b)
			(new Thread() { 
				public void run() {
					setListener(true);
				} 
			}).start();
		else
			(new Thread() { 
				public void run() {
					setListener(false);
				} 
			}).start();
    }
    
    private void setListener(boolean b) {
    		if (b) {
			if (listener == null) {
				if (initRecvSocket()) {
					listener = new Thread(this);
					listener.start();
				}
			}
		} else {
			if (recvSocket != null) {
			recvSocket.close();
			}
			listener = null;
		}
    }

    private boolean initRecvSocket() {
		try {
			recvSocket = new DatagramSocket(port);
			return true;
		}
		catch (BindException be) {
			MaxSystem.error(debugString + ": there is already an object bound to port "+port);
		} catch (SocketException se) {
			MaxSystem.error(debugString+": socket exception: "+se);
		} 
		return false;
    }

    public void run() {
		while (true) {
			try {
				byte[] buf = new byte[1024];
				DatagramPacket packet = new DatagramPacket(buf, buf.length);
				recvSocket.receive(packet);
				callback.setArgs(new Object[]{Atom.parse(new String(packet.getData(),0,packet.getLength(),"UTF-8"),false)});
				callback.execute();
			} catch (IOException ie) {
				// toggleListener closes the socket to force loop to exit
				return;
			}
		}
    }
    
    /**
     * Set the data receiving callback method.  
     * The method must take an array of Atoms 
     * as its only argument.
     * @param toCallIn the object that contains the method
     * @param methodName the name of the method to call
     */
    public void setCallback(Object toCallIn, String methodName) {
		callback = new Callback(
					toCallIn, 
					methodName, 
					new Object[] {new Atom[] {} }
					);	
    }
    
    /**
     * Should be called when the UdpReceiver is no longer needed.
     */
    public void close() {
    		setActive(false);
    }
}
