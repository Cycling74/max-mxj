package com.cycling74.net;
import com.cycling74.max.*;
import java.net.*;
import java.io.IOException;
import java.util.*;

/**
 * Connect to and receive from multicast groups.  
 * An internal Callback is maintained from a specified 
 * method name and object.  The specified method must 
 * take an array of Atoms as its only argument.  When
 * data is received, it is translated to an array of
 * Atoms and passed to the specified method.  When a
 * MultiReceiver is no longer needed its close() method 
 * should be called.
 * 
 * created on 18-May-2004
 * @author Ben Nevile
 */
public class MultiReceiver implements Runnable {
	private static Class ATOM_ARRAY_CLASS = (new Atom[] {}).getClass();
	private String debugString = "MultiReceiver";
	private int port;
	private boolean active = false;

    private MulticastSocket mRecvSocket;
    private Thread mListener = null;
	private boolean keepRunning;
	private Callback callback = null;
	private Vector addresses = new Vector();


	/**
	 * inner class to handle joining groups
	 */
	private class MultiJoiner extends Thread {
		String name;
		
		MultiJoiner(String name) {
			this.name = name;
		}
		
		public void run() {
			try {
				InetAddress addr = InetAddress.getByName(name);
				mRecvSocket.joinGroup(addr);
			} catch (UnknownHostException uhe) {
				MaxSystem.error(debugString+": unknown host " + name + ". "+uhe);
			} catch (SocketException se) {
				MaxSystem.error(debugString+": socket error. "+se);
				MaxSystem.post("Have you already joined that group?");
			} catch (IOException ie) {
				MaxSystem.error(debugString+": io error. "+ie);
			}
		}
	}
	
	/**
	 * inner class to handle leaving groups
	 */
	private class MultiLeaver extends Thread {
		String name;
		
		MultiLeaver(String name) {
			this.name = name;
		}
		
		public void run() {
			try {
				InetAddress addr = InetAddress.getByName(name);
				mRecvSocket.leaveGroup(addr);
			} catch (UnknownHostException uhe) {
				MaxSystem.error(debugString+": unknown host " + name + ". "+uhe);
			} catch (SocketException se) {
				MaxSystem.error(debugString+": socket error. "+se);
				MaxSystem.post("Are you sure you were a member of that group?");
			} catch (IOException ie) {
				MaxSystem.error(debugString+": io error. "+ie);
			}
		}
	}

    /**
     * Creates a new MultiReceiver.  
     * To make anything happen the user will have to
     * connect to a port, group, and set a Callback.
     */
    public MultiReceiver() {
    }
	
    /**
     * Creates a new MultiReceiver connected to a specified group and port, 
	 * and creates an internal callback to a given method.
     * @param groupName the group to join
     * @param port port to communicate over
     * @param toCallIn object that contains the method to call
     * @param methodName method to call
     */
    public MultiReceiver(String groupName, 
						int port, 
						Object toCallIn,
						String methodName) {
    	setCallback(toCallIn, methodName);
    	addAddress(groupName);
    	setPort(port);
    }
    

    /**
     * Creates a new MultiReceiver connected to a specified group and port. 
     * A callback must be set with setCallback before this new MultiReceiver
     * will do anything.
     * @param groupName the group to join
     * @param port port to communicate over
     */
    public MultiReceiver(String groupName, int port) {
    	addAddress(groupName);
    	setPort(port);
    }
    
    /**
     * Creates a new MultiReceiver connected to a specified port.
     * A callback must be set with setCallback and a group joined
     * with join before this new MultiReceiver will do anything.
     * @param port port to communicate over
     */
    public MultiReceiver(int port) {
    	setPort(port);
    }
    
   
    /**
     * Sets the port to listen to.  
     * This requires closing the old
     * socket and creating a new one.  
     * This method operates asynchronously.
     * @param port the new port
     */
    public void setPort(int port) {
    	this.port = port;
    	resetListener();
    }
    
    /**
     * @return the current port
     */
    public int getPort() {
    	return port;
    }
    
    /**
     * Creates and maintains a Callback for the MultiReceiver.  
     * This Callback is what is called when data is received.
     * The method must take an array of Atoms as its only argument.
     * @param toCallIn object that contains the method
     * @param methodName method to call
     */
    public void setCallback(Object toCallIn, String methodName) {
    		callback = new Callback(
						toCallIn, 
						methodName, 
						new Object[] {new Atom[] {} }
						);	
	}
    
    private void addAddress(String name) {
    	synchronized(addresses) {
    		addresses.add(name);
    	}
    }
	
	/**
	 * Turn the MultiReceiver on or off.
	 * @param b true to turn it on
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
			if (mListener == null) {	
				keepRunning = true;
				if (initRecvSocket()) {
					joinAllGroups();
					mListener = new Thread(this);
					mListener.start();
					active = true;
				}
			}
		} else {
			if (mListener != null) {
				keepRunning = false;
			}
			if (mRecvSocket != null) {
				mRecvSocket.close();
			}
			mListener = null;
			active = false;
		}
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
     * @return an array of Strings representing the groups joined, one for each group
     */
    public String[] getGroups() {
    	return (String[])addresses.toArray(Atom.emptyArray);
    }
    
    private void joinAllGroups() {
    	synchronized(addresses) {
    	      Iterator i = addresses.iterator(); 
    	      while (i.hasNext()) {
    	      	MultiJoiner jl = new MultiJoiner((String)i.next());
    	      	jl.start();
    	      }
    	  }
    }

    /**
     * Join a multicast group.
     * @param name the name of the group to join.  eg 224.74.74.74 (the maxhole group)
     */
    public void join(String name) {
    	addAddress(name);
    	if (active) {
    		MultiJoiner jl = new MultiJoiner(name);
    		jl.start();
    	}
    }

    private boolean initRecvSocket() {
    	try {
    		mRecvSocket = new MulticastSocket(port);
    	} catch (IOException ioe) {
    		MaxSystem.error(debugString+": io error. "+ioe);
    	}
    	return true;
    }

    public void run() {
    	byte[] buf = new byte[1024];
    	DatagramPacket packet = new DatagramPacket(buf, buf.length);
    	while (keepRunning) {
	    	try {
				mRecvSocket.receive(packet);
				if (keepRunning) {
					if (callback != null) {
						String s = new String(packet.getData(), 0,  packet.getLength(), "UTF-8");
						callback.setArgs(new Object[] {Atom.parse(s)});
						callback.execute();
					}
				}
			} catch (IOException ie) {
				// the socket closed.  this is OK.
			}
    	}
    }
   
   
    /**
     * Close down the MultiReceiver.  Should be called in your notifyDeleted method.
     */
    public void close() {
    	//I'm not really sure why I decided to start a new thread here.  
    	//at some point I have to go through this stuff and make sure
    	//I'm not doing something stupid.
    	(new Thread() {
			public void run() {
    	    		leaveAllGroups();
			}
		}).start();
    }
    
    /**
     * Leave all groups.
     */
    public void leaveAllGroups() {
    	synchronized(addresses) {
    		Iterator i = addresses.iterator(); 
    		while (i.hasNext()) {
    			MultiLeaver jl = new MultiLeaver((String)i.next());
    			jl.start();
    		}
    		addresses.clear();
    	}
    }
    
    /**
     * Leave one group.
     * @param name the group to leave
     */
    public void leave(String name) {
    	boolean found = false;
    	synchronized(addresses) {
    		int count = 0;
    		Iterator i = addresses.iterator(); 
  	      	while ((i.hasNext())&&(!found))
  	      		if (((String)i.next()).equals(name))
  	      			found = true;
  	      		else 
  	      			count++;
  	      	if (found) 
  	      		addresses.remove(count);
    	}
    	if (found) {
    		MultiLeaver jl = new MultiLeaver(name);
        	jl.start();
    	}
    }
    
    /**
     * Sets the debug string 
     * to be used for purposes of error reporting.
     * 
     * @param name the debug string to use when error reporting
     */
    public void setDebugString(String name) {
    		this.debugString = name;
    }
}


