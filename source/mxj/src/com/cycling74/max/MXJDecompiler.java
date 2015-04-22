package com.cycling74.max;

import jode.decompiler.*;

/**
*We currently use JODE http://jode.sourceforge.net as 
* the backend for this interface.
*@author Topher LaFata
*/

public class MXJDecompiler
{

	private static Decompiler _decompiler = null;
	//can't instantiate
	private MXJDecompiler(){};
	/**
	*Get the singleton instance of the MXJDecompiler.
	*/	
	public static MXJDecompiler getInstance()
	{
		if(_decompiler == null)
			_decompiler =  new Decompiler();
		return new MXJDecompiler();	
	}
	
	//these calls might need to be synchronized on a lock
	//classname needs to be fully qualified in . format i.e. java.lang.String
	/**
	*Attempt to decompile a class. Not all classes can be decompiled to valid source
	* for various reasons but most simple classes can. The current MaxSystem.getClassPath
	* is used to search for the class you wish to decompile.
	*@param classname the name of the class you wish to decompile in dot format i.e. com.cycling74.max.Atom
	*@param writer any subclass of writer to where the results of decompilation will be dumped.
	*/
	public void decompile(String classname, java.io.Writer writer) throws java.io.IOException
	{
		decompile(classname, MaxSystem.getClassPath(), writer);
	}
	/**
	* same as above but you can specify the classpath yourself as an array of strings.
	*/
	public void decompile(String classname,String[] classpath,java.io.Writer writer) throws java.io.IOException
	{
		try{
			//this part is to make sure we could decompile and object when the class may not have been
			//loaded from the classpath proper but from the default path. -tml
			String extra = MXJClassLoader.getInstance().getCodeSource(classname);
			if(extra != null)
			{
				if(extra.endsWith(".class"))
				{
					extra = extra.substring(0,extra.lastIndexOf(java.io.File.separatorChar));
				}
				
				String[] classpath2 = new String[classpath.length + 1];
				System.arraycopy(classpath,0,classpath2,0,classpath.length);
				classpath2[classpath.length] = extra;
				classpath = classpath2;
			}
		}catch(Exception e){}
		
		_decompiler.setClassPath(classpath);
    	_decompiler.decompile(classname,writer,null);		
	}


}