package com.cycling74.max;


import java.io.File;
import java.util.Vector;
import java.util.HashMap;

;

/**
*If you are looking at this class I am imagining that the method names are more
* or less self-explanatory.
*@author Topher LaFata
*/

public class MXJClassLoader
{

	private static MXJClassLoader _me     = new MXJClassLoader();
	private MXJClassLoaderImpl _instance  = null;
	private HashMap _extended_class_cache = null;
	private HashMap _cs_map               = null;
	
	Vector   _v_classpath = null;
	private String _extended_class_search_dir = null; 
	private boolean _zapped = false;
	
	private MXJClassLoader()
	{
		_v_classpath = new Vector(16);
		_extended_class_cache = new HashMap(64);
		_cs_map = new HashMap(64);
	}
	
	public static MXJClassLoader getInstance()
	{
		return _me;
	}


	public String[] getCurrentClassPath()
	{
		return _instance.getCurrentClassPath();
	}
	
	public void dump()
	{
		_instance.dump();
	}

	
	class modinfo
	{
		File f;
		long initialmod;
		modinfo(File f_, long lastmodified_)
		{
			f = f_;
			initialmod = lastmodified_;
		}
	}

	public Class loadClazz(String name, boolean resolve, boolean report_error)
	{
		Class c = null;
		String class_code_source = null; 
		File f = null;
		int i;
		

		if(_instance == null)
			_instance = new MXJClassLoaderImpl(_v_classpath,_extended_class_search_dir);
		
		try{
			modinfo mi = (modinfo)_cs_map.get(name);
			if((mi != null && mi.f.lastModified() > mi.initialmod) || _zapped)
			{
				c = (new MXJClassLoaderImpl(_v_classpath,_extended_class_search_dir)).doLoadClass(name,resolve,report_error);
				_cs_map.remove(name);
				_extended_class_cache.remove(name);
					//System.out.println("(mxj) loaded "+name+"from new classloader");
				_zapped = false;
			} 		
			else
			{
				if((c = (Class)_extended_class_cache.get(name)) != null)
					return c;
				c = _instance.doLoadClass(name,resolve,report_error);
			}
			

			 
			if(_cs_map.get(name) == null)
			{
				try{
					class_code_source = c.getProtectionDomain().getCodeSource().getLocation().getFile();
			
					if(!class_code_source.endsWith(".jar"))
					{
						class_code_source = class_code_source + name.replace('.', File.separatorChar)+".class";
					}
					f = new File(class_code_source);
					_cs_map.put(name,new modinfo(f,f.lastModified()));			
				}catch(NullPointerException npe)
				{
					//system class
				}
			}
			
			if(_extended_class_cache.get(name) == null)
				_extended_class_cache.put(name,c);
			
		}catch(ClassNotFoundException e)
		{
			return null;
		}
		return c;
	}
	
	protected String getCodeSource(String classname)
	{
		modinfo i  = (modinfo)_cs_map.get(classname);
		if(i == null)
			return null;
		return i.f.getAbsolutePath();
	}
				
    protected void addDirectory(String dirname)
    {
		if(!_v_classpath.contains(dirname))
			_v_classpath.addElement(dirname);
		//_instance.addDirectory(dirname);
    }

	protected void setExtendedClassSearchDirectory(String dirname)
	{
		_extended_class_search_dir = dirname;
		if(_instance != null)
			_instance.setExtendedClassSearchDirectory(dirname);
	}

	protected String[] getDynamicClassPath()
 	{
 	
		return getCurrentClassPath();
 	}
 
	protected void zap()
	{
		_zapped = true;
	}
}

