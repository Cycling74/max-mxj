package com.cycling74.net;
import com.cycling74.max.*;
import java.net.*;
import java.io.*;

/**
 *
 * TCP receiver.
 * An internal Callback is maintained from a specified 
 * method name and object.  The specified method must 
 * take an array of Atoms as its only argument.  When
 * data is received, it is translated to an array of
 * Atoms and passed to the specified method.  When a
 * TcpReceiver is no longer needed its close() method 
 * should be called.
 * 
 * created on 24-May-2004
 * @author Ben Nevile
 */
public class TcpReceiver implements Runnable {

    private int port;
    private ServerSocket recvSocket;
    private Thread listener = null;
    private String debugString = "TcpReceiver";
    private Callback callback = null;
    private boolean shouldRun = true;
    
    /**
     * Creates a TcpReceiver with specified port, receiving method name,
     * and containing object.
     * @param port the port over which to receive data
     * @param toCallIn the object tnat contains the receivnig method
     * @param methodName the name of the receiving method
     */
    public TcpReceiver(int port, Object toCallIn, String methodName) {
    	setCallback(toCallIn, methodName); 
	 if (port!=0) 
	 	setPort(port);
    }
    
    /**
     * Creates a TcpReceiver.  Port, receiving method name and
     * containing object must be specified before anything will happen.
     */
    public TcpReceiver() {
    	this(0, null, null);
    }
    
    /**
     * Creates a TcpReceiver with specified port.  A receiving method name and
     * containing object must be specified before anything will happen.
     * @param port
     */
    public TcpReceiver(int port) {
    	this(port, null, null);		
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
				try {
				    recvSocket.close();
				} catch (IOException ie) {
					MaxSystem.error(debugString+": io exception: "+ie);
				}
		    }
		    listener = null;
		}
    }
    
    private boolean initRecvSocket() {
    	if (port!=0) {
			try {
			    recvSocket = new ServerSocket(port);
			    return true;
			} catch (BindException be) {
				MaxSystem.error(debugString+": there is already an object " +
												"bound to port "+port);
			} catch (SocketException se) {
			    MaxSystem.error(debugString+": socket exception: "+se);
			} catch (IOException ie) {
				MaxSystem.error(debugString+": io exception: "+ie);
			}
    	}
		return false;
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
     * Turns the receiver on or off.
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
    
    /**
     * Set the name of the debug string.
     * @param debugString this String will be part of the message
     * printed to the Max window in the event of a networking failure.
     */
    public void setDebugString(String debugString) {
    	this.debugString = debugString;
    }
    
    /**
     * Set the port.
     * @param port the port to receive data over
     */
    public void setPort(int port) {
		if (port != this.port) {
			this.port = port;
			resetListener();
		}
    }
 
    /**
     * Gets the port.
     * @return the port data is being received over
     */
    public int getPort() {
    	return port;
    }
     
   /**
    * Must be called when the TcpReceiver is no longer needed.  
    * Should probably be called from your notifyDeleted() method.
    */
    public void close() {
    	shouldRun = false;
        if (listener != null) {
           try {
                recvSocket.close();
            } catch (IOException ie) {
            	MaxSystem.error(debugString+": io exception: "+ie);
            }
        }
    }
    
    public void run() {
		while (shouldRun) { //is always executing while the thread is active
		    try {
				recvSocket.setSoTimeout(50);
				Socket socket = recvSocket.accept();
				BufferedReader input =
					new BufferedReader(
		            	new InputStreamReader(
		            		socket.getInputStream(), "UTF-8"));
				while (shouldRun) {
				    String msg = input.readLine();
				    if (msg == null) {
						input.close();
						break;
				    }
				    if (callback != null) {
				    	callback.setArgs(new Object[] {Atom.parse(msg)});
					    callback.execute();
				    }
				}
		    } catch (SocketTimeoutException se) {
		    	
		    } catch (IOException ie) {
				// toggleListener closes the socket to force loop to exit
				return;
		    }
		}
	}
    
    /**
     * Sets the receiving callback.  The receiving method must accept
     * an array of Atoms as its only argument.
     * @param toCallIn object that contains the receiving method.
     * @param methodName name of the method to receive data
     */
    public void setCallback(Object toCallIn, String methodName) {
    	if ((toCallIn != null)&&(methodName != null)) {
			callback = new Callback(
						toCallIn, 
						methodName, 
						new Object[] {new Atom[] {} }
						);	
    	}
    }
}
