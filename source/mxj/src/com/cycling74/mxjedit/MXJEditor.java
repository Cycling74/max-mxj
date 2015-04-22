package com.cycling74.mxjedit;

import com.cycling74.max.*;
import javax.swing.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.event.*;
import java.util.*;

public class MXJEditor extends JFrame 
{
	

    private static final String UNTITLED = "Untitled";
    private JEditTextArea _editor = null;
    private JMenuBar      _menubar = null;

    private JMenu _fmenu; //File menu
    private JMenu _emenu; //Edit menu
    private JMenuItem _mode_menu;
    //JAVA MODE MENUS
    private JMenuItem _mode_menu_compile;
	private JMenuItem _mode_menu_findclass;
    
    private JMenuItem _fmenu_new;
    private JMenuItem _fmenu_open;
    private JMenuItem _fmenu_save;
    private JMenuItem _fmenu_saveas;
    private JMenuItem _fmenu_close;
    
    private JMenuItem _emenu_undo;
    private JMenuItem _emenu_redo;
    private JMenuItem _emenu_cut;
    private JMenuItem _emenu_copy;
    private JMenuItem _emenu_paste;
    private JMenuItem _emenu_find;
    private JMenuItem _emenu_gotoline;
    private JMenuItem _emenu_findagain;
    private JMenuItem _emenu_findselected;
    
    private JMenuItem _fontmenu_elements[] = null;

    private File _current_file = null; //File user is currently editing
    private String _current_search_term;
    private int _current_mode;

	//these are accelerator keys we want to respond to in the apple menu.
	private static final char[] __accs = new char[]{'X','C','V','S','Z','O','N','W','F','G','H','L','K','D','E'};				
	//E is for toggling between edit window and the compile window. We need to register it here since that compile dialog
	// is a child of ours
	
	private Callback saveCallback = null;
	
	public MXJEditor(Callback saveCallback)
	{
		this();
		this.saveCallback = saveCallback;
	}
    
    public MXJEditor()
    {
	setTitle(UNTITLED);
	_menubar = new JMenuBar();

	//File Menu
	_fmenu        = new JMenu("File");

	_fmenu_new    = new JMenuItem("New");
	_fmenu_new.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
	_fmenu_new.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent evt)
		{
		    menu_new();
		}
	    });
	_fmenu_open   = new JMenuItem("Open...");
	_fmenu_open.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
	_fmenu_open.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent evt)
		{
		    menu_open();
		}
	    });
	_fmenu_save   = new JMenuItem("Save");
	_fmenu_save.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
	_fmenu_save.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent evt)
		{
		    menu_save();
		}
	    });

	_fmenu_saveas = new  JMenuItem("Save As...");
	_fmenu_saveas.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent evt)
		{
		    menu_saveas();
		}
	    });
	_fmenu_close  = new JMenuItem("Close");
	_fmenu_close.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_W,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
	_fmenu_close.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent evt)
		{
		    menu_close();
		}
	    });

	_fmenu.add(_fmenu_new);
	_fmenu.add(_fmenu_open);
	_fmenu.add(_fmenu_save);
	_fmenu.add(_fmenu_saveas);
	_fmenu.add(_fmenu_close);

	_menubar.add(_fmenu);

	//Edit Menu
	_emenu           = new JMenu("Edit");

	_menubar.add(_emenu);
	_emenu.addMenuListener(new MenuListener()
	    {
		public void menuSelected(MenuEvent e)
		{
		    _emenu_undo.setEnabled(_editor.getUndoManager().canUndo());
		}
		public void menuDeselected(MenuEvent e){};
		public void menuCanceled(MenuEvent e){};
	    }
			       );
	
	_emenu_undo       = new JMenuItem("Undo");
    _emenu_undo.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Z,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
    _emenu_undo.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent evt)
                {
                    menu_undo();
                }
            });
	_emenu_redo       = new JMenuItem("Redo");
        _emenu_redo.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _emenu_redo.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent evt)
                {
                    menu_redo();
                }
            });

	_emenu_cut       = new JMenuItem("Cut");
	_emenu_cut.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_X,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
	_emenu_cut.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent evt)
		{
		    menu_cut();
		}
	    });
	_emenu_copy      = new JMenuItem("Copy");
	_emenu_copy.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_C,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
	_emenu_copy.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent evt)
		{
		    menu_copy();
		}
	    });
	_emenu_paste     = new JMenuItem("Paste");
	_emenu_paste.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_V,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _emenu_paste.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent evt)
                {
                    menu_paste();
                }
            });
	_emenu_find      = new JMenuItem("Find...");
        _emenu_find.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _emenu_find.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent evt)
                {
                    menu_find();
                }
            });
	_emenu_findagain = new JMenuItem("Find Again");
        _emenu_findagain.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_G,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _emenu_findagain.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent evt)
                {
                    menu_findagain();
                }
            });

	_emenu_findselected = new JMenuItem("Find Selected");
        _emenu_findselected.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_H,KeyEvent.SHIFT_MASK | Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _emenu_findselected.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent evt)
                {
                    menu_findselected();
                }
            });

	_emenu_gotoline  = new JMenuItem("Goto Line...");
	_emenu_gotoline.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_L,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _emenu_gotoline.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent evt)
                {
                    menu_gotoline();
                }
            });


	_emenu.add(_emenu_undo);
	//_emenu.add(_emenu_redo); ..fuck the redo
	_emenu.add(_emenu_cut);
	_emenu.add(_emenu_copy);
	_emenu.add(_emenu_paste);
	_emenu.add(_emenu_find);
	_emenu.add(_emenu_findagain);
	_emenu.add(_emenu_findselected);
	_emenu.add(_emenu_gotoline);
	
	//the below commented out for now, 
	//but it would be great to have a full-fledged Font menu 
	//at some point in the future...
	/*
	_fontmenu = new JMenu("Font");
	//populate a list of all the fonts on the system
	Font[] allfonts = GraphicsEnvironment.getLocalGraphicsEnvironment().getAllFonts();
	//Method setFontMeth = resolveMethod(this, "setFont", new Object[] {allfonts[0]});
	Method setFontMeth=null;
	try {
		setFontMeth = this.getClass().getDeclaredMethod("setFont", new Class[] {allfonts[0].getClass()});
	} catch (Exception e) {}
	_fontmenu_elements = new JMenuItem[allfonts.length];
	for (int i=0;i<_fontmenu_elements.length;i++)
	{
		_fontmenu_elements[i] = new JMenuItem(allfonts[i].getName());
		_fontmenu_elements[i].addActionListener(new FontActionListener(this, setFontMeth, allfonts[i]));
		_fontmenu.add(_fontmenu_elements[i]);
	}

	_menubar.add(_fontmenu);
	*/

	setJMenuBar(_menubar);
	
	//make editor
	_editor = new JEditTextArea(this);
	getContentPane().setLayout( new BorderLayout() );
	getContentPane().add( _editor, BorderLayout.CENTER );
	addWindowListener( new WindowAdapter(){
	    public void windowClosing(WindowEvent e)
		{
		    setVisible(false);
			dispose();
		}
	    });
	//setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
	this.setTabSize(4);

    setSize(new Dimension(640,480));
    
    //do this before you make window visible.
	com.cycling74.max.MaxSystem.registerCommandAccelerators(__accs);
	
	Dimension dim = getToolkit().getScreenSize();
	setLocation(dim.width / 3, 20);
	//setVisible(true);
	_editor.requestFocus();
    }
    
    //add the rest of the supported modes at some point
    public static final int MODE_JAVA       = 1;
    public static final int MODE_DEFAULT    = 2;
    public static final int MODE_JAVASCRIPT = 3;
    public static final int MODE_GLSL 		= 4;
    public static final int MODE_CFUNK      = 5;
    public void setMode(int mode)
    {
    	switch(mode)
    	{
    		case MODE_JAVA:
    			if(_current_mode != MODE_JAVA)
    				_clear_mode_menu();	
    			
    			_editor.setTokenMarker(new JavaTokenMarker());
    			if(_mode_menu == null)
    			{	

    
    				_mode_menu = new JMenu("Java");    				
    				_mode_menu_compile = new JMenuItem("Open Compile Window...");
    				_mode_menu_compile.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_K,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
    				_mode_menu_compile.addActionListener(new ActionListener()
    				{
    					public void actionPerformed(ActionEvent evt)
    					{
    						menu_java_compile();
    					}
    				});

    					
    				_mode_menu_findclass = new JMenuItem("Open Class Reference for Selected Class...");
    				_mode_menu_findclass.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_D,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
    				_mode_menu_findclass.addActionListener(new ActionListener()
    				{
    					public void actionPerformed(ActionEvent evt)
    					{
    						menu_java_findclass();
    					}
    				});
    				
    			_mode_menu.add(_mode_menu_compile);
    			_mode_menu.add(_mode_menu_findclass);
    			getJMenuBar().add(_mode_menu);			
    			}
    		
   				_current_mode = mode;
    			break;
    		case MODE_JAVASCRIPT:
    		    if(_current_mode != MODE_JAVASCRIPT)
    				_clear_mode_menu();
    			_editor.setTokenMarker(new JavaScriptTokenMarker());
    			if(_mode_menu == null)
    			{
    				_mode_menu = new JMenu("JavaScript");
    				JMenuItem _mode_menu_check_syn = new JMenuItem("Check Syntax");
    				_mode_menu_check_syn.addActionListener(new ActionListener()
    				{
    					public void actionPerformed(ActionEvent evt)
    					{
    						System.out.println("Do Syntax check!!(Not implemented)");
    					}
    				});
    				_mode_menu.add(_mode_menu_check_syn);
    				getJMenuBar().add(_mode_menu);	
    			}
    			_current_mode = mode;
    			break;	
    		case MODE_GLSL:
    			if (_current_mode != MODE_GLSL)
    				_clear_mode_menu();
    			_editor.setTokenMarker(new GLSLTokenMarker());
    			if (_mode_menu != null)
    			{
    				getJMenuBar().remove(_mode_menu);
    				_mode_menu = null;
    			}
    			_current_mode = mode;
    			break;
			case MODE_CFUNK:
    			if (_current_mode != MODE_CFUNK)
    				_clear_mode_menu();
    			_editor.setTokenMarker(new CTokenMarker());
    			if (_mode_menu == null)
    			{
    				
					_mode_menu = new JMenu("CFunk");    				
    				_mode_menu_compile = new JMenuItem("Open Compile Window...");
    				_mode_menu_compile.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_K,Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
    				_mode_menu_compile.addActionListener(new ActionListener()
    				{
    					public void actionPerformed(ActionEvent evt)
    					{
    						menu_cfunk_compile();
    					}
    				});
					_mode_menu.add(_mode_menu_compile);
					getJMenuBar().add(_mode_menu);	
    			}
    			_current_mode = mode;
    			break;	
    		case MODE_DEFAULT:
    			if(_mode_menu != null)
    			{
    				getJMenuBar().remove(_mode_menu);
    				_mode_menu = null;
    			}
    			_current_mode = mode;
    		default:
    			_editor.setTokenMarker(null);//default
    			break;//do nothing
    	}
   
    }
    
    // the below commented out for now - bbn
    /*
    public void setFont(Font f) 
    {
    	//this.setFont(f);
    }
    */
    
    private void _clear_mode_menu()
    {
    	if(_mode_menu != null)
    	{
    		getJMenuBar().remove(_mode_menu);
    		_mode_menu = null;
    	}
    }
    private void _do_set_buffer(String s)
    {
    	setEditable(true);
    	_editor.setText(s);
		_editor.scrollTo(0,0);
	    _editor.select(0,0);
    }
    
    public void setBuffer(String st)
    {	
		_do_set_buffer(st);
    }
    
    public void setBufferFromFile(String filename)
    {
	File f;

	f = new File(filename);
	if(f.exists())
	    setBufferFromFile(f);
	else
	    System.err.println("File "+filename+" not found!");
	
    }

    public void setBufferFromFile(File f)
    {
	
		StringBuffer buf = new StringBuffer(2048);
		String s = null;
	
	try{
		//change this to read(buffer, blah, blah
		// and read file by bytes insted of readline
	    BufferedReader br = new BufferedReader(new FileReader(f));
	    setTitle("loading "+f.getAbsolutePath());
	    while((s = br.readLine()) != null)
		{
		    buf.append(s+"\n");
		}
	    _do_set_buffer(buf.toString());
	     set_current_file(f); 
		
		   
	    
	    if(f.getName().endsWith(".java"))
	    	setMode(MODE_JAVA);
		else if(f.getName().endsWith(".js"))
	    	setMode(MODE_JAVASCRIPT);
		else if ((f.getName().endsWith(".jxs"))||(f.getName().endsWith("glsl")))
			setMode(MODE_GLSL);
		else if ((f.getName().endsWith(".cfunk.c")))
			setMode(MODE_CFUNK);	
		else
			setMode(MODE_DEFAULT);
			
	} catch(FileNotFoundException fne)
	    {
		System.err.println("File "+f.getName()+" not found!");
	    }
	catch(IOException ioe){
	    ioe.printStackTrace();
	}		 

	
    }
    
	public void setEditable(boolean editable)
	{
		//hack!!
		if(_current_mode == MODE_JAVA)
    		_mode_menu_compile.setEnabled(editable);
	
		_editor.editable = editable;
	}

	public boolean isEditable()
	{
		return _editor.isEditable();
	}
    public void setTabSize(int tabsize)
    {
        _editor.getDocument().putProperty(PlainDocument.tabSizeAttribute, 
					  new Integer(tabsize));
    }


    private void menu_new()
    {
		MXJEditor e = new MXJEditor();
    	java.awt.Point p = this.getLocationOnScreen();
    	e.setLocation(p.x + 5,p.y +5);
    	e.setVisible(true);
    }

    private void menu_open()
    {
    	FileDialog fd = new FileDialog(this);
    	fd.setVisible(true);
    	String filename = fd.getFile();//null if cancel
    	if(filename != null)
    	 	setBufferFromFile(fd.getDirectory()+filename);
    }
	//this is protected so that the compile frame can call back when the compile button is pushed.
    protected void menu_save()
    {
		if(getTitle().equals(UNTITLED) || !isEditable())//force save as for read only files
	    	menu_saveas();
		else
	    	do_save(_current_file);
		if (saveCallback != null)
			saveCallback.execute();
    }

    private void menu_saveas()
    {
		FileDialog fd = new FileDialog(this,"Save As..",FileDialog.SAVE);
		if(_current_file != null)
		{
			fd.setDirectory(_current_file.getPath());
			fd.setFile(_current_file.getName());
		}
		else
		{
			fd.setDirectory("");//default to user.dir 	
		}
		fd.setVisible(true);
		String filename = fd.getFile();//null if cancel
    	if(filename != null)
			do_save(new File(fd.getDirectory()+filename));     	
    }

    private void do_save(File f)
    {   
    	int idx;
		String ap = f.getAbsolutePath();
		//File dir = f.getParentFile();
		//System.out.println("dir is "+dir.getAbsolutePath());			
		//if(!dir.exists())
		//dir.mkdirs();
		
		try{
			//we have to use
			f.getParentFile().mkdirs();
			//System.out.println("parent is "+f.getParentFile().getAbsolutePath());
			BufferedWriter out = new BufferedWriter(new FileWriter(f,false));
		    String s = null;
		    for(int i = 0; i < _editor.getLineCount();i++)
			{
			    s = _editor.getLineText(i);
			    out.write(s+"\n",0,s.length() + 1);
			}
		    out.close();
		    set_current_file(f);
			setEditable(true);
		//setBufferFromFile(f);
		}catch(IOException ioe)
		    {
				ioe.printStackTrace();
				System.err.println("(MXJEditor) Save Failed: "+f.getAbsolutePath()+"\n"+ioe.getMessage());
				menu_saveas();
		    }
    }
    


    private void menu_close()
    {
		this.setVisible(false);
    	this.dispose();
    }


    private void menu_cut()
    {
		_editor.cut();
    }

    private void menu_copy()
    {
		_editor.copy();
    }

    private void menu_paste()
    {
		_editor.paste();
    }
    private void menu_gotoline()
    {	    
		com.cycling74.max.MaxSystem.nextWindowIsModal();
    	SwingUtilities.invokeLater(new Runnable(){
    		public void run()
    		{
    			int i;
    			int doclen = _editor.getLineCount();
    			try{
	    			i = Integer.parseInt((String)JOptionPane.showInputDialog(MXJEditor.this,"Goto Line",
	    																	"Goto Line:",JOptionPane.PLAIN_MESSAGE));
	    		if(i < 1)
					i = 1;
	    		else if(i > doclen)
					i = doclen;
	    		_editor.setCaretPosition(_editor.getLineStartOffset(i-1));
		    	}catch(NumberFormatException nfe){}
			}
		});
    }
    
    public void gotoLine(int l)
    {
		int doclen = _editor.getLineCount();	
		if(l < 1)
			l = 1;
		else if(l > doclen)
			l = doclen;
		_editor.setCaretPosition(_editor.getLineStartOffset(l-1));

    }
    
    public void selectLine(int l)
    {
	
		int doclen = _editor.getLineCount();	
		if(l < 1)
			l = 1;
		else if(l > doclen)
			l = doclen;    
   		
   		_editor.select(_editor.getLineStartOffset(l-1),_editor.getLineEndOffset(l-1)-1);
    }

    public void setCurrentSearchTerm(String s)
    {
	_current_search_term =s;
    }

    public void menu_find()
    {
		com.cycling74.max.MaxSystem.nextWindowIsModal();
    	SwingUtilities.invokeLater(new Runnable(){
			public void run()
			{
					String def;
					if(_current_search_term != null)
						def = _current_search_term;
					else
						def = "";	
					String s = (String)JOptionPane.showInputDialog(MXJEditor.this,"Find","Find:",JOptionPane.PLAIN_MESSAGE, null,
                    null,def);
					if(s != null)
					{
	    				SyntaxDocument doc = _editor.getDocument();
	    				do_find(s,_editor.getCaretPosition(),doc.getLength(),true);
    				}
    		}
    	});
    }

    public void menu_findagain()
    {
    SyntaxDocument doc = _editor.getDocument();
	if(_current_search_term != null)
	    do_find(_current_search_term,_editor.getCaretPosition(),doc.getLength(),true);
	else
	    beep();
    }

    public void menu_findselected()
    {
	String selection = _editor.getSelectedText();
	SyntaxDocument doc = _editor.getDocument();
	if(selection == null)
		return;
	if( selection.length() > 1)
	    {
		if(do_find(selection,_editor.getCaretPosition(),doc.getLength(),true ))
		    setCurrentSearchTerm(selection);
		else
		    beep();
	    }
    }

    private boolean do_find(String s,int from_offset, int to_offset,boolean wrap)
    {
	if(s != null)
	    {
		s = s.trim();
		setCurrentSearchTerm(s);
		SyntaxDocument doc = _editor.getDocument();
		int offs = from_offset;
		//int nleft = doc.getLength() - offs;
		int nleft = to_offset - offs;
		Segment text = new Segment();
		text.setPartialReturn(true);   
		char[] term = new char[s.length()];
		s.getChars(0,s.length(),term,0);
		int t_len = term.length;
		
		boolean inmatch = false;
		int i = 0;
		int abscount = offs;
	
		//wrapping = true;
		try{
		    char c;
		    int t = 0;
		    String doc_text = doc.getText(offs,nleft);	
		    for(i = 0; i < doc_text.length();i++)
			{
			    c = doc_text.charAt(i);
			    //System.err.println("chr at i("+i+"): "+c+" char at t("+t+") is "+term[t]);
			    if(c == term[t])
				{
				    t++;
				    if(t == ( t_len))
					{
					    _editor.select(i - (t_len - 1) + offs,i+offs+1);
					    return true;
					}
				}
			    else
				{
				    t = 0;
				}

			}
		    
		}catch(Exception e)
		    {
				e.printStackTrace();
		    	return false;
		    }
		if(wrap)
		{
			return do_find(s,0,from_offset +1,false);
		}
		else
		{	
			beep();
			return false;
	    }  
	}//if s != null
	
	return false;  
    
}
    private void menu_undo()
    {
	if(_editor.getUndoManager().canUndo())
	    _editor.getUndoManager().undo();
	else
	    beep();
	//  _emenu_undo.setEnabled(false);
    }

    private void menu_redo()
    {
	if(_editor.getUndoManager().canRedo())
	    _editor.getUndoManager().redo();
   
    }

	private ClassLoader findClassLoaderForContext(MXJEditCompileContext c) {
		ClassLoader context = Thread.currentThread().getContextClassLoader();
		ClassLoader me = c.getClass().getClassLoader();
		ClassLoader system = ClassLoader.getSystemClassLoader();
		return (context == null) ? (me == null) ? system : me : context;
	}
	
	
	//these should only get called when in java mode
	private MXJEditCompileFrame _compile_frame = null;
	private void menu_java_compile()
	{
		menu_save();	
		
		if(_current_file != null)
		{
			String f_sep = System.getProperty("file.separator");
			String p_sep = System.getProperty("path.separator");

			String[] cp = MaxSystem.getClassPath();
			StringBuffer cpath = new StringBuffer();
			for(int i = 0; i < cp.length;i++)
			{
				cpath.append(cp[i]);
				if(i != cp.length -1)
					cpath.append(p_sep);
			}
			
			final MXJEditCompileContext ctx = new MXJEditCompileContext();
			ctx.setSourceFile(_current_file.getAbsolutePath());
			ctx.setSourceDirectory(_current_file.getAbsoluteFile().getParent());
			//ctx.setBuildDirectory(MXJPreferences.getMXJCompilerBuildRoot());
			
			//always display the build directory as one to one with the source file.
			ctx.setBuildDirectory(_current_file.getAbsoluteFile().getParent());		
			ctx.setCompileCommand(MXJPreferences.getMXJCompilerCommmand());
			ctx.setClassPath(cpath.toString());
			ctx.setEditor(this);
	
			if(_compile_frame != null)
			{
				SwingUtilities.invokeLater(new Runnable()
				{
					public void run()
					{
						_compile_frame.setCompileContext(ctx);
						_compile_frame.setVisible(true);
					}
				});	
			}
			else
			{
				SwingUtilities.invokeLater(new Runnable()
				{
					public void run()
					{
						// due to a Java race condition bug in JEditorPane.registerEditorKitForContentType
						// we need to ensure we have a valid classloader - if this ever causes classloader
						// issues with our own custom classloader, we might want to consider a different fix
						// (we could catch NullPointerExceptions when creating the Compile window and
						// retry opening it in the catch block)
						Thread thread = Thread.currentThread();
						ClassLoader old = thread.getContextClassLoader();
						thread.setContextClassLoader(findClassLoaderForContext(ctx));
						try {
							_compile_frame = new MXJEditCompileFrame(ctx);
						} finally {
							MaxSystem.post("reset classloader");
							thread.setContextClassLoader(old);
						}
					}
				});
			}
		}
	}
	
		//these should only get called when in java mode
	private MXJEditCfunkCompileFrame _cf_compile_frame = null;
	private void menu_cfunk_compile()
	{
		menu_save();	
		
		if(_current_file != null)
		{
			String f_sep = System.getProperty("file.separator");
			String p_sep = System.getProperty("path.separator");
			
			final MXJEditCompileContext ctx = new MXJEditCompileContext();
			ctx.setSourceFile(_current_file.getAbsolutePath());
			ctx.setSourceDirectory(_current_file.getAbsoluteFile().getParent());
			ctx.setEditor(this);
	
			if(_cf_compile_frame != null)
			{
				SwingUtilities.invokeLater(new Runnable()
				{
					public void run()
					{
						_cf_compile_frame.setCompileContext(ctx);
						_cf_compile_frame.setVisible(true);
					}
				});	
			}
			else
			{
				SwingUtilities.invokeLater(new Runnable()
				{
					public void run()
					{
						_cf_compile_frame = new MXJEditCfunkCompileFrame(ctx);
					}
				});
			}
		}
	}
	

	
	private boolean _is_whitespace(char c)
	{
		return (Character.isWhitespace(c) || c == '\n' || c == '\r');
	}
	void menu_java_findclass()
	{
		
		String classname = _editor.getSelectedText();
		if(classname == null)
			return;
		//grab package prefixes
		HashSet prefixes = new HashSet();
		String  line = null;
		prefixes.add("java.lang");
		//This shit should be done with regular expressions at some point -tml
		for(int i = 0;i < _editor.getLineCount();i++)
		{
			line = _editor.getLineText(i).trim();	
			if(line.equals(""))
				continue;
			else if(line.startsWith("/") ||
					line.startsWith("*"))
						continue;//comment
			else if(line.startsWith("import"))
			{	
				int dot_idx  = line.lastIndexOf('.');
				line = (line.substring(0,dot_idx).split(" "))[1].trim();
				prefixes.add(line);
				continue;
			}
			else if(line.startsWith("package"))
			{	
				int semi_idx  = line.lastIndexOf(';');
				line = (line.substring(0,semi_idx).split(" "))[1].trim();
				prefixes.add(line);
				continue;
			}
			else if(line.length() == 1)
			{
				if(_is_whitespace(line.charAt(0)))
					continue;			
			}
			//this is a hackish attempt to detect crappy newlines like \n\r
			else if(line.length() == 2)
			{
				if(_is_whitespace(line.charAt(0)) &&
					_is_whitespace(line.charAt(1)))
					continue;			
			}
			else
				break;
			
		}
		
		
		Class c 	= null;
		Iterator it = prefixes.iterator();
		String prefix = null;
		while(it.hasNext())
		{
			prefix = (String)it.next();
			c = MXJClassLoader.getInstance().loadClazz(prefix+"."+classname,false,false);
			if(c != null)
				break;

		}
		
		if(c == null)//if c still equals null try global package
		{	
				c = MXJClassLoader.getInstance().loadClazz(classname,false,false);
				if(c == null)
				{
					beep();
					return;
				}
		}
	/*NOW WE ALWAYS SHOW THE CLASS REF AND DON"T OPEN THE SOURCE BECAUSE IT IS SORT
	OF CONFUSING. MAYBE MOVE THIS CODE INTO A FIND_SOURCE MENU ITEM */
	/*	boolean show_ref = true;
		java.security.CodeSource cs =  c.getProtectionDomain().getCodeSource();	
		String abspath = null;
		if(cs != null)
		{
			java.net.URL url = c.getProtectionDomain().getCodeSource().getLocation(); 
			String dir = url.getFile();
		
			if((dir.substring(dir.length() - 1,dir.length())).equals(System.getProperty("file.separator")))
				{
					show_ref = false;
					abspath = dir+(c.getName()).replace('.','/')+".java";	
				}
		}
		
		if(show_ref)//System class..bootstrap loader.Just display ref..
		{
	*/
			final Class cc = c;
			SwingUtilities.invokeLater(new Runnable()
    		{
    			public void run()
    			{
    				MXJClassRefFrame f = new MXJClassRefFrame(cc);
    			}
    	
    		});
    				
	/*	}				
		else//show source
		{
			final File f = new File(abspath);
			if(!f.exists())
    		{	//We need to create the java file!
    			System.out.println("(mxj) Unable to locate "+abspath+". Decompiling with JODE");
    			//final StringWriter source = new java.io.StringWriter(2048);
    			try{
    				BufferedWriter bw = new BufferedWriter(new FileWriter(f)); 
    				MXJDecompiler.getInstance().decompile(c.getName(),MaxSystem.getClassPath(),bw);
    			}catch(IOException e)
    			{
    				System.err.println("(mxj) Unable to decompile "+c.getName());
    				System.err.println("message: "+e.getMessage());
    				return;
    			}
   
    		}		
 
    		   SwingUtilities.invokeLater
			(
				new Runnable()
					{
						public void run()
						{
						    	java.awt.Point p = getLocationOnScreen();
								MXJEditor e  = new MXJEditor();
								e.setLocation(p.x + 5,p.y +5);
								e.setBufferFromFile(f);
    							e.setVisible(true);							
								
						}
					}
			);	
    		
    		//System.out.println("DELETE: found class at base url "+url.toString());
    		//System.out.println("DELETE: found class at base url as file "+url.getFile());
   		}//end show source	 	
	*/
	}
	
	///END JAVA MODE FUCTIONS
	public void setCurrentFile(File f)
	{
		set_current_file(f);
	}
	
    private void set_current_file(File f)
    {
		String s = f.getAbsolutePath();
		_current_file = f;
		setTitle(s);
    }

    public static void main(String[] args)
    {
	MXJEditor e = new MXJEditor();
	//e.setBufferFromFile(args[0]);
    }
	

    private void beep()
    {
	Toolkit.getDefaultToolkit().beep();
    }
    
    protected void finalize()
    {
    	//System.err.println("Editor "+this+" is collected");
    }

	public void setVisible(boolean vis)
	{
		super.setVisible(vis);
		if(vis)
			_editor.requestFocus();		
	} 

}
