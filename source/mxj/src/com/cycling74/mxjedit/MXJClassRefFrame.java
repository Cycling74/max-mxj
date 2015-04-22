package com.cycling74.mxjedit;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.lang.reflect.*;
import java.util.Arrays;
import java.util.Comparator;

//we should probably implement cacheing in here at some point
public class MXJClassRefFrame extends JFrame
{
    JTree _ref;
    DefaultTreeModel _tree_model;
	private static final Color DEFAULT_COLOR = new Color(0xFFFFFF);

	    
    public MXJClassRefFrame(Class c)
    {
		super(c.getName());
		DefaultMutableTreeNode root    = new DefaultMutableTreeNode(c.getName());
		DefaultMutableTreeNode fields_node  = new DefaultMutableTreeNode("fields");
		DefaultMutableTreeNode methods_node = new DefaultMutableTreeNode("methods");

		Method[] methods = c.getDeclaredMethods();
	

  		Arrays.sort(methods, new Comparator() {
    		public int compare(Object o1, Object o2) {
      			String s1 = ((Method)o2).getName();
      			String s2 = ((Method)o1).getName();
      			return s2.compareTo(s1);
    		}
  		});

	
		StringBuffer info = new StringBuffer(32);
		Class[] params = null;
		Class return_type = null;
		Method m = null;
		int modifiers = 0;
		int the_meth_cnt = methods.length;
		for(int i = 0; i < methods.length;i++)
	    {
			m = methods[i];
			modifiers = m.getModifiers();

			if((modifiers & Modifier.PRIVATE) != 0 ||
				m.getName().indexOf('$') != -1) //get rid of inner class stuff
			{//ignore private methods
		 		the_meth_cnt--;
		    	continue;
			}
			if((modifiers & Modifier.PUBLIC) != 0)
		   	 info.append("public ");
			if((modifiers & Modifier.STATIC) != 0)
		    	info.append("static ");
			if((modifiers & Modifier.ABSTRACT) != 0 )
		    	info.append("abstract ");
			if((modifiers & Modifier.NATIVE) != 0)
		    	info.append("native ");

			if((modifiers & Modifier.PROTECTED) != 0)
		    	info.append("protected ");
			if((modifiers & Modifier.SYNCHRONIZED) != 0)
		    	info.append("synchronized ");


			params = m.getParameterTypes();
			return_type = m.getReturnType();
			info.append(_get_short_classname(return_type)+" ");
			info.append(m.getName()+"(");
			for(int p = 0; p < params.length;p++)
		    {
				info.append(_get_short_classname(params[p]));
				if(p != params.length -1)
			    	info.append(",");
		    }
		
			info.append(")");
			methods_node.add(new DefaultMutableTreeNode(info.toString()));
			info.setLength(0);
	    }
	
		Field[] fields = c.getDeclaredFields();
		Class field_type = null;
		int the_field_cnt = fields.length;
		for(int i = 0; i < fields.length;i++)
	    {
			field_type = fields[i].getType();
			modifiers = fields[i].getModifiers();

			if((modifiers & Modifier.PRIVATE) != 0||
				fields[i].getName().indexOf('$') != -1)
			{
		    	the_field_cnt--;
		    	continue;
			}
			if((modifiers & Modifier.PUBLIC) != 0)
		    	info.append("public ");
			if((modifiers & Modifier.STATIC) != 0)
		    	info.append("static ");

			if((modifiers & Modifier.PROTECTED) != 0)
		    info.append("protected ");
		
			if((modifiers & Modifier.VOLATILE) != 0)
		    	info.append("volatile ");
			if((modifiers & Modifier.TRANSIENT) != 0)
		    	info.append("transient ");
			if((modifiers & Modifier.FINAL) != 0)
		    	info.append("final ");

			info.append(_get_short_classname(field_type)+" ");
			info.append(fields[i].getName());
			fields_node.add(new DefaultMutableTreeNode(info.toString()));
			info.setLength(0);
	    }	      
		if(!(the_field_cnt <= 0))
			root.add(fields_node);
		if(!(the_meth_cnt <= 0))
			root.add(methods_node);
		_tree_model = new DefaultTreeModel(root);
		_ref = new JTree(_tree_model);
	
	JScrollPane p = new JScrollPane(_ref);
	p.setBackground(DEFAULT_COLOR);
	getContentPane().add(p,BorderLayout.CENTER);
	
	_ref.addKeyListener(new KeyAdapter()
	    {
			public void keyPressed(KeyEvent evt)
			{	    
		    	int keycode = evt.getKeyCode();
		    	int mod = evt.getModifiers();
		    	if(keycode == KeyEvent.VK_W &&
		       	(mod & Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()) != 0)
				{
			    	setVisible(false);
			    	dispose();
				}
		}

	    });

		setDefaultCloseOperation(DISPOSE_ON_CLOSE);
		pack();
		_ref.setBackground(DEFAULT_COLOR);//match mxj editor for now
		
		setSize(300,400);
		setVisible(true);
    }
    


    private String _get_short_classname(Class c)
    {
	String classname = c.getName();
	boolean is_array = false;
	if(classname.charAt(0) == '[')
	    {
		switch(classname.charAt(1))
		    {
		    case 'L':
			is_array = true;
			break;
		    case 'Z':
			return "boolean[]";
		    case 'B':
			return "byte[]";
		    case 'C':
			return "char[]";
		    case 'S':
			return "short[]";
		    case 'I':
			return "int[]";
		    case 'J':
			return "long[]";
		    case 'F':
			return "float[]";
		    case 'D':
			return "double[]";

			
		    }

	    }

	int idx = classname.lastIndexOf('.');
	if(idx != -1)
		classname = classname.substring(idx + 1,classname.length());
	if(is_array)
	    classname = (classname.substring(0,classname.length() - 1))+"[]";//remove semi colon from  [Ljava.lang.blah;
		

	return classname;

    }
    
    public static void main(String[] args)
    {
	String classname = args[0];
	try{
	    Class c = Class.forName(classname);
	    new MXJClassRefFrame(c);
	}catch(Exception e)
	    {
		e.printStackTrace();
	    }
    }
    
}
