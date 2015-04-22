package com.cycling74.max;

public class AttributeInfo
{
	public String name = null;
	public String j_f_type = null;;
	public String m_f_type = null;
	
	public String setter = null;
	public String setter_jptypes = null;
	public String setter_sig = null;
	
	public String getter = null;
	public String getter_jptypes = null;
	public String getter_sig = null;
	
	public boolean settable  = false;
	public boolean gettable  = false;
	public int     isvirtual = 1;//we may overrdie this in the future to differentiate bewtween object attributes and virtual attributes

	
	public String toString()
	{
		StringBuffer b = new StringBuffer();
		b.append("name = "+name+"\n");
		b.append("jftype = "+j_f_type+"\n");
		b.append("mftype = "+m_f_type+"\n");
		b.append("setter = "+setter+"\n");
		b.append("setter jptypes = "+setter_jptypes+"\n");
		b.append("setter_sig = "+setter_sig+"\n");
		b.append("getter = "+getter+"\n");
		b.append("getter jptypes = "+getter_jptypes+"\n");
		b.append("getter_sig = "+getter_sig+"\n");
		b.append("settable = "+settable+"\n");
		b.append("gettable = "+gettable+"\n");
		b.append("isvirtual = "+isvirtual+"\n");
		return b.toString();
	}
	
	AttributeInfo()
	{

	}


}
