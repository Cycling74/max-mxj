/*
 * JavaTokenMarker.java - Java token marker
 * Copyright (C) 1999 Slava Pestov
 *
 * You may use and modify this package for any purpose. Redistribution is
 * permitted, in both source and binary form, provided that this notice
 * remains intact in all source distributions of this package.
 */
package com.cycling74.mxjedit;


import java.io.*;
import java.util.zip.*;
import java.util.*;
import com.cycling74.max.*;

/**
 * Java token marker.
 *
 * @author Slava Pestov
 * @version $Id$
 */
public class JavaTokenMarker extends CTokenMarker
{
	public JavaTokenMarker()
	{
		super(false,getKeywords());
	}

	public static KeywordMap getKeywords()
	{
		if(javaKeywords == null)
		{
			javaKeywords = new KeywordMap(false);
			javaKeywords.add("package",Token.KEYWORD2);
			javaKeywords.add("import",Token.KEYWORD2);
			javaKeywords.add("byte",Token.KEYWORD3);
			javaKeywords.add("char",Token.KEYWORD3);
			javaKeywords.add("short",Token.KEYWORD3);
			javaKeywords.add("int",Token.KEYWORD3);
			javaKeywords.add("long",Token.KEYWORD3);
			javaKeywords.add("float",Token.KEYWORD3);
			javaKeywords.add("double",Token.KEYWORD3);
			javaKeywords.add("boolean",Token.KEYWORD3);
			javaKeywords.add("void",Token.KEYWORD3);
			javaKeywords.add("class",Token.KEYWORD3);
			javaKeywords.add("interface",Token.KEYWORD3);
			javaKeywords.add("abstract",Token.KEYWORD1);
			javaKeywords.add("final",Token.KEYWORD1);
			javaKeywords.add("private",Token.KEYWORD1);
			javaKeywords.add("protected",Token.KEYWORD1);
			javaKeywords.add("public",Token.KEYWORD1);
			javaKeywords.add("static",Token.KEYWORD1);
			javaKeywords.add("synchronized",Token.KEYWORD1);
			javaKeywords.add("native",Token.KEYWORD1);
			javaKeywords.add("volatile",Token.KEYWORD1);
			javaKeywords.add("transient",Token.KEYWORD1);
			javaKeywords.add("break",Token.KEYWORD1);
			javaKeywords.add("case",Token.KEYWORD1);
			javaKeywords.add("continue",Token.KEYWORD1);
			javaKeywords.add("default",Token.KEYWORD1);
			javaKeywords.add("do",Token.KEYWORD1);
			javaKeywords.add("else",Token.KEYWORD1);
			javaKeywords.add("for",Token.KEYWORD1);
			javaKeywords.add("if",Token.KEYWORD1);
			javaKeywords.add("instanceof",Token.KEYWORD1);
			javaKeywords.add("new",Token.KEYWORD1);
			javaKeywords.add("return",Token.KEYWORD1);
			javaKeywords.add("switch",Token.KEYWORD1);
			javaKeywords.add("while",Token.KEYWORD1);
			javaKeywords.add("throw",Token.KEYWORD1);
			javaKeywords.add("try",Token.KEYWORD1);
			javaKeywords.add("catch",Token.KEYWORD1);
			javaKeywords.add("extends",Token.KEYWORD1);
			javaKeywords.add("finally",Token.KEYWORD1);
			javaKeywords.add("implements",Token.KEYWORD1);
			javaKeywords.add("throws",Token.KEYWORD1);
			javaKeywords.add("this",Token.LITERAL2);
			javaKeywords.add("null",Token.LITERAL2);
			javaKeywords.add("super",Token.LITERAL2);
			javaKeywords.add("true",Token.LITERAL2);
			javaKeywords.add("false",Token.LITERAL2);
		
			//add clases from max.jar
			_add_maxjar_to_keywords();
		
		}
		return javaKeywords;
	}

	// private members
	private static KeywordMap javaKeywords;

	private static void _add_maxjar_to_keywords()
	{
			String maxjar = MaxSystem.locateFile("max.jar");
			if(maxjar != null)
			{
				ZipFile zf = null;
				File f = new File(maxjar);
				if(!f.exists())
					return;
				try{
					zf = new ZipFile(f);
				}catch(IOException e)
				{
					return;
				}
				Enumeration e = zf.entries();	
			
				String classname = null;
				int dotidx = 0;
				while(e.hasMoreElements())
				{
					classname = ((ZipEntry)e.nextElement()).getName();
					
					if(!classname.endsWith(".class"))
						continue;//doesn't have a class extension..could be .java file or something else
					else
						dotidx = classname.lastIndexOf(".class");						
					classname = classname.substring(0,dotidx);//remove .class part
					classname = classname.replace('/','.');//change slashes to dots
					javaKeywords.add(classname,Token.KEYWORD2);
					
					if((dotidx = classname.lastIndexOf(".")) == -1)
						continue;//top level package
					classname = classname.substring(dotidx + 1); //remove package part
					
					javaKeywords.add(classname,Token.KEYWORD2);
				}
			
			}

	}
}
