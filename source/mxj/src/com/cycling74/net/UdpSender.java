// created by bbn on May 23, 2004

package com.cycling74.net;

import java.net.*;
import java.io.IOException;
import com.cycling74.max.*;

/**
 * UDP sender.
 *
 * @author Ben Nevile
 */
public class UdpSender {
    private InetAddress iAddress;
    private String address = null;
    private int port = 0;
    private DatagramSocket sendSocket;
    private boolean goodParams = false;
    String objectName = "UdpSender";

    /**
     * Creates a UdpSender with specified IP address and port.
     * @param address IP address to send data to
     * @param port port to send data to
     */
    public UdpSender(String address, int port) {
		this.address = address;
		this.port = port;
		initSendSocket(address, port);
    }
    
    /**
     * Creates a UdpSender.  IP address and port must be specified
     * before anything can be sent.
     */
    public UdpSender() {	
    }

    /**
     * Set port.
     * @param port the port to send data to
     */
    public void setPort(int port) {
		if (port != this.port) {
    		this.port = port;
    		if (address != null) {
    			initSendSocket(address, port);
    		}
		}
    }
    
    /**
     * Get port.
     * @return the port data is being sent to
     */
    public int getPort() {
    		return port;
    }
    
    /**
     * Set IP address.
     * @param address the IP address to send data to
     */
    public void setAddress(String address) {
		// only change if it's different from the previous one
		if (!address.equals(this.address)) {
				this.address = address;
				if (port != 0) {
					initSendSocket(address, port);
				}
		}
    }
    
    /**
     * Get IP address.
     * @return the IP address data is being sent to
     */
    public String getAddress() {
    		return address;
    }
    
    private void initSendSocket(String addrarg, int portarg) {
		try {
			goodParams = false;
			iAddress = InetAddress.getByName(addrarg);
			port = portarg;
			sendSocket = new DatagramSocket();
			goodParams = true;
		} catch (UnknownHostException uhe) {
			MaxSystem.error(objectName+": Unknown host: "+addrarg);
		} catch (SocketException se) {
			MaxSystem.error(objectName+": Socket exception: "+se);
		} 
    }	

    private void send(String msg) {
		if (goodParams) {
			try {
				byte[] b = msg.getBytes("UTF-8");
				DatagramPacket packet = new DatagramPacket(b, b.length, iAddress, port);
				sendSocket.send(packet);
			} catch (IOException ie) {
				MaxSystem.error(objectName+": IOException: "+ie);
			}
		} else {
			MaxSystem.error(objectName + "not ready to send.");
		}
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
     * Send a list.
     * @param a the array of Atoms to send.
     */
    public void send(Atom[] a) {
    		send(Atom.toOneString(a));
    }
    /**
     * Send a message.
     * @param msg the message to send
     * @param a arguments to the message
     */
    public void send(String msg, Atom[] a) {
    		send(msg + " " + Atom.toOneString(a));
    }
}