
/*
 * Copyright (c) 2001-2003, Cycling '74.  All rights reserved.
 */

package com.cycling74.io;

import java.io.*;
import java.util.StringTokenizer;



/**
 * A factory
 * that returns <tt>PrintStream</tt> objects that write to the Max console.
 *
 * @see java.io.PrintStream
 *
 * @author Herb Jellinek
 */

abstract class ConsoleStream extends PrintStream {
public static final String NL = "\n";
    protected static final int MAX_LINE_LENGTH = 200;

    private ByteArrayOutputStream mBaos = null;

    protected ConsoleStream(ByteArrayOutputStream baos) {
	super(baos, true);
	mBaos = baos;
    }

    /**
     * The method to use to send output to the console - will typically
     * call either post or error.
     */
    protected abstract void send(String s);

//prints

   public void print(float f)
    {
 		String s = String.valueOf(f);	   
    	mBaos.write(s.getBytes(),0, s.length());
    }
    
   public void print(long l)
    {
    	String s = String.valueOf(l);	   
    	mBaos.write(s.getBytes(),0, s.length());
    }
    
   public void print(int i)
    {
    	String s = String.valueOf(i);	   
    	mBaos.write(s.getBytes(),0, s.length());
    }

    public void print(String s)
    {
    	mBaos.write(s.getBytes(),0, s.length());
    }
    
    
   public void print(Object o)
    {
        String s;
        
        if(o != null)
    		s = o.toString();	   
    	else
    		s = "null";	
    	mBaos.write(s.getBytes(),0, s.length());
    	
    }
    
   public void print(double d)
    {
    	String s = String.valueOf(d);	   
    	mBaos.write(s.getBytes(),0, s.length());
    }
    
   public void print(char[] c)
    {
    	String s = String.valueOf(c);	   
    	mBaos.write(s.getBytes(),0, s.length());
    }
    
   public void print(char c)
    {
    	String s = String.valueOf(c);	   
    	mBaos.write(s.getBytes(),0, s.length());
    }

    public void print(boolean b)
    {
    	String s = String.valueOf(b);	   
    	mBaos.write(s.getBytes(),0, s.length());
    }
///printlns

 	public void println()
 	{
 		flush();
 	}
 		
   	public void println(float f)
    {
 		String s = String.valueOf(f);	   
    	mBaos.write(s.getBytes(),0, s.length());
    	flush();
    }
    
   public void println(long l)
    {
    	String s = String.valueOf(l);	   
    	mBaos.write(s.getBytes(),0, s.length());
    	flush();
    }
    
   public void println(int i)
    {
    	String s = String.valueOf(i);	   
    	mBaos.write(s.getBytes(),0, s.length());
   		flush();
    }

    public void println(String s)
    {
    	mBaos.write(s.getBytes(),0, s.length());
    	flush();
    }
    
    
   public void println(Object o)
    {
    	String s;
    	if(o != null)
    		s = o.toString();	   
    	else
    		s = "null";
    	mBaos.write(s.getBytes(),0, s.length());
        flush();	
    
    }
    
   public void println(double d)
    {
    	String s = String.valueOf(d);	   
    	mBaos.write(s.getBytes(),0, s.length());
        flush();
    }
    
   public void println(char[] c)
    {
    	String s = String.valueOf(c);	   
    	mBaos.write(s.getBytes(),0, s.length());
        flush();
    }
    
   public void println(char c)
    {
    	String s = String.valueOf(c);	   
    	mBaos.write(s.getBytes(),0, s.length());
        flush();
    }

    public void println(boolean b)
    {
    	String s = String.valueOf(b);	   
    	mBaos.write(s.getBytes(),0, s.length());
        flush();
    }


    /**
     * Flush the output to the console.
     */
    public void flush() {
	super.flush();
	String s = mBaos.toString();
	int len = s.length();
	StringTokenizer st = new StringTokenizer(s, NL);

	while (st.hasMoreTokens()) {
	    String line = st.nextToken();
	    int linelength = line.length();
	    if (line.length() > MAX_LINE_LENGTH) {
		for (int i = 0; i < linelength; i += MAX_LINE_LENGTH) {
		    String sub =
			line.substring(i,
				       Math.min(i + MAX_LINE_LENGTH, 
						linelength));
		    send(sub);
		}
	    } else {
		send(line);
	    }
	}
	mBaos.reset();
    }

    /**
     * Reset the output buffer.
     */
    public void reset() {
	super.flush();
	mBaos.reset();
    }
}
