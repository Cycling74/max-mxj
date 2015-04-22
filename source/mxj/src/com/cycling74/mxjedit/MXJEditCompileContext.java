
package com.cycling74.mxjedit;


public class MXJEditCompileContext
{
    private String _sourcefile      = null;
    private String _sourcedirectory = null;
    private String _compile_command = null;
    private String _build_directory = null;
    private String _classpath       = null;
	private MXJEditor _editor       = null;    
    public MXJEditCompileContext()
    {
		
    }
    
    public void setSourceFile(String sf)
    {
		_sourcefile = sf;
    }

    public String getSourceFile()
    {
		return _sourcefile;
    }

    public void setSourceDirectory(String sd)
    {
		_sourcedirectory = sd;
    }

    public String getSourceDirectory()
    {
		return _sourcedirectory;
    }

    public void setCompileCommand(String cc)
    {
		_compile_command = cc;
    }

    public String getCompileCommand()
    {
	return _compile_command;
    }

    public void setBuildDirectory(String bd)
    {
		_build_directory = bd;
    }
    public String getBuildDirectory()
    {
		return _build_directory;
    }

    public void setClassPath(String cp)
    {
		_classpath = cp;
    }

    public String getClassPath()
    {
		return _classpath;
    }

	public void setEditor(MXJEditor editor)
	{
		_editor = editor;
	}
	
	public MXJEditor getEditor()
	{
		return _editor;
	}
}
