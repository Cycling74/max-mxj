package com.cycling74.mxjedit;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.io.*;
import javax.swing.text.*;
import java.awt.Color;
import com.cycling74.max.MXJPreferences;

public class MXJEditCfunkCompileFrame extends JFrame
{

    ColorPane messages = null;
    
    JButton compile_button = null;
    JButton close_button = null;
    private MXJEditCompileContext _ctx = null;
	    
	private CaretListener _error_lineno_dispatch = new CaretListener(){	
						public void caretUpdate(CaretEvent e) 
						{
							int mark = e.getMark();
							int dot  = e.getDot();
							
							int lineno = -1;
							if(mark != dot)//selection
							{
								try{
									ColorPane cp = (ColorPane)e.getSource();
									lineno = Integer.parseInt(cp.getSelectedText());
									MXJEditCfunkCompileFrame cf = cp.compile_frame;
									MXJEditCompileContext ctx = cf.getCompileContext();
									MXJEditor ed = ctx.getEditor();
									if(!ed.isVisible())
										ed.setVisible(true);
									else
										ed.toFront();
									ed.selectLine(lineno);
								}catch(NumberFormatException nfe)
								{
									
								}
								
								//setSelectionStart(mark);
								//setSelectionEnd(mark);
							}

						}
					};    
    
    public MXJEditCfunkCompileFrame(MXJEditCompileContext ctx)
    {

		_ctx = ctx;	
		//messages panel
		messages = new ColorPane(this);
		messages.setEditable(false);

		JScrollPane jsp = new JScrollPane(messages);
		//Action Panel
		JPanel ap = new JPanel();
		compile_button = new JButton("Compile");
		close_button   = new JButton("Close");

		compile_button.addActionListener(new ActionListener(){
			public void actionPerformed(ActionEvent e)
			{
				//Do the compile in a different thread so that we can still have the statys window updating.
				Thread t = new Thread(new Runnable()
				{
					public void run()
					{
						if(compile_button.isEnabled())
							compile_button.setEnabled(false);
						do_compile();
						if(!compile_button.isEnabled())
							compile_button.setEnabled(true);
					}
				});
			t.start();
			}
		}); 

	close_button.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent e)
		{
		    do_close();
		}
	    });

		
	ap.add(compile_button);
	ap.add(close_button);
	getRootPane().setDefaultButton(compile_button);
	
	this.addKeyListener(new KeyAdapter()
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
	this.addKeyListener(new KeyAdapter()
	    	{
	    	public void keyPressed(KeyEvent evt)
				{	    
		    		int keycode = evt.getKeyCode();
		    		int mod = evt.getModifiers();
		    		if(keycode == KeyEvent.VK_E &&
		       		(mod & Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()) != 0)
					{
						MXJEditor e = _ctx.getEditor();
						if(e.isVisible())
							e.toFront();
						else
							e.setVisible(true);	

					}
				}

	    });   
	    
	////////Put it together man
	getContentPane().add(jsp,BorderLayout.CENTER);
	getContentPane().add(ap,BorderLayout.SOUTH);
	
	setTitle("MXJ Funklet Compile Window");
	pack();
	setSize(750,320);
	Dimension dim = getToolkit().getScreenSize();
	this.setLocation(dim.width / 4, 50);
	setVisible(true);
	requestFocus();
    
    }

    public void setCompileContext(MXJEditCompileContext ctx)
    {
    	_ctx = ctx;
	}
    
    public MXJEditCompileContext getCompileContext()
    {
    	return _ctx;
    }


    protected void do_compile()
    {
   
    	final Color ERROR_COLOR = Color.red.darker();
    	final Color MESSAGE_COLOR = Color.blue.darker();
    	final Color OK_COLOR = Color.green.darker();

 		messages.setEditable(true);
    	messages.setText(null);
		    	
		String sourcefile = _ctx.getSourceFile();
		File sourcedir = new File(_ctx.getSourceDirectory());
		if(!sourcedir.exists() || !sourcedir.isDirectory())
		{
			messages.append(ERROR_COLOR,"sourcedirectory "+sourcedir.getAbsolutePath()+" does not exist or is not a directory");
			return;
		}
		boolean error = false;
		setSize(750,320);

		if(_ctx.getEditor() != null)
			_ctx.getEditor().menu_save();
		try
		{ 
			messages.removeCaretListener(_error_lineno_dispatch);
			String[] cmd = new String[2];
			cmd[0] = "make";
			cmd[1] = "-k";
			String env[] = null;//new String[]{"LD_PREBIND=0"};
			Runtime rt = Runtime.getRuntime();
			Process proc = rt.exec(cmd,env,sourcedir);
					
			InputStream stderr = proc.getErrorStream();
			InputStreamReader isr = new InputStreamReader(stderr);
			BufferedReader br = new BufferedReader(isr);
			
			final BufferedReader stdout = new BufferedReader(new InputStreamReader(proc.getInputStream()));
			
			
			messages.append(MESSAGE_COLOR,"Compiling "+sourcefile+".........\n\n");
			
			messages.append(MESSAGE_COLOR,"executing ");
			for(int i=0; i < cmd.length;i++)
				messages.append(MESSAGE_COLOR,cmd[i]+" ");
			messages.append(MESSAGE_COLOR," in directory "+sourcedir.toString());
			messages.append(MESSAGE_COLOR,"\n\n");
			
			Thread t = new Thread()
			{
				public void run()
				{
					String line;
					try{
						while ( (line = stdout.readLine()) != null)
						{
							messages.append(MESSAGE_COLOR,line+"\n");
						}
					}catch(IOException e){}
				}
			};
			t.start();
				
			String line = null;
			String err_lineno   = null;
			String err_msg      = null;
			String err_rest_msg = null; 
			int err_count = 0;
			int idx = -1;
				
			while ( (line = br.readLine()) != null)
			{
			
				if((idx = line.indexOf("funk.c")) != -1)
				{
					err_count++;
					//try and parse out error line num
					int first_col  = -1;
					int second_col = -1;
						
					first_col  = line.indexOf(':',idx);
					second_col = line.indexOf(':',first_col + 1);
					if(first_col == -1 || second_col == -1)
					{	//just do the normal thing. the error message seems messed up or is not SUN
						messages.append(ERROR_COLOR,line+"\n");
					}
					else
					{
						err_msg = line.substring(0,first_col);
						messages.append(ERROR_COLOR,err_msg);
						err_lineno = line.substring(first_col +1, second_col);
						messages.append(MESSAGE_COLOR,"[ "+err_lineno+" ]",14);
						err_rest_msg = line.substring(second_col +1);
						messages.append(ERROR_COLOR,err_rest_msg+"\n");
					}
				}
				else
				{
					messages.append(ERROR_COLOR,line+"\n");
				}
			}
		    	
			String now = java.text.DateFormat.getDateTimeInstance().format(new java.util.Date()); 
			int exitVal = proc.waitFor();
			if(exitVal == 0)
			{
				messages.append(OK_COLOR,"0 errors\n");
				messages.append(OK_COLOR,"[ "+now+" ]\n");
				messages.append(OK_COLOR," compilation of "+sourcefile+" was successful");	
				try{
					Thread.sleep(1500);
				}catch(InterruptedException e){}
					this.setVisible(false);
					this.dispose();			
				}
				else
				{
					int err_scale = 25;//50 pixels in frame height gained per error
					messages.append(ERROR_COLOR,"[ "+now+" ]\n");
					messages.append(ERROR_COLOR," compilation of "+sourcefile+" failed.");		
					setSize(getWidth(), 320 + (int)(err_scale * Math.min(8,err_count)));
					//add this so they can click on the text and get brought to the line in the source file
					messages.addCaretListener(_error_lineno_dispatch);
				}
				proc.destroy();
		
	    	} catch (Throwable t)
			{
				messages.append(ERROR_COLOR,"Unexpected Error During Compile:\n\n");
		    	messages.append(ERROR_COLOR,"    "+t.getMessage()+"\n\n");
		    	messages.append(ERROR_COLOR,"See Max console for stack trace.\n");
		    	t.printStackTrace();
			}

 	messages.setEditable(false);
    }

    private void do_close()
    {
		this.setVisible(false);
		this.dispose();
    }

/////////////////////////LAYOUT CRAP...


    /**
     * Aligns the first <code>rows</code> * <code>cols</code>
     * components of <code>parent</code> in
     * a grid. Each component in a column is as wide as the maximum
     * preferred width of the components in that column;
     * height is similarly determined for each row.
     * The parent is made just big enough to fit them all.
     *
     * @param rows number of rows
     * @param cols number of columns
     * @param initialX x location to start the grid at
     * @param initialY y location to start the grid at
     * @param xPad x padding between cells
     * @param yPad y padding between cells
     */
    public static void makeCompactGrid(Container parent,
                                       int rows, int cols,
                                       int initialX, int initialY,
                                       int xPad, int yPad) {
        SpringLayout layout;
        try {
            layout = (SpringLayout)parent.getLayout();
        } catch (ClassCastException exc) {
            System.err.println("The first argument to makeCompactGrid must use SpringLayout.");
            return;
        }

        //Align all cells in each column and make them the same width.
        Spring x = Spring.constant(initialX);
        for (int c = 0; c < cols; c++) {
            Spring width = Spring.constant(0);
            for (int r = 0; r < rows; r++) {
                width = Spring.max(width,
                                   getConstraintsForCell(r, c, parent, cols).
				   getWidth());
            }
            for (int r = 0; r < rows; r++) {
                SpringLayout.Constraints constraints =
		    getConstraintsForCell(r, c, parent, cols);
                constraints.setX(x);
                constraints.setWidth(width);
            }
            x = Spring.sum(x, Spring.sum(width, Spring.constant(xPad)));
        }

        //Align all cells in each row and make them the same height.
        Spring y = Spring.constant(initialY);
        for (int r = 0; r < rows; r++) {
            Spring height = Spring.constant(0);
            for (int c = 0; c < cols; c++) {
                height = Spring.max(height,
                                    getConstraintsForCell(r, c, parent, cols).
				    getHeight());
            }
            for (int c = 0; c < cols; c++) {
                SpringLayout.Constraints constraints =
		    getConstraintsForCell(r, c, parent, cols);
                constraints.setY(y);
                constraints.setHeight(height);
            }
            y = Spring.sum(y, Spring.sum(height, Spring.constant(yPad)));
        }

        //Set the parent's size.
        SpringLayout.Constraints pCons = layout.getConstraints(parent);
        pCons.setConstraint(SpringLayout.SOUTH, y);
        pCons.setConstraint(SpringLayout.EAST, x);
    }
   
   public void setVisible(boolean vis)
	{
		super.setVisible(vis);
		if(vis){
			this.requestFocus();		
			getRootPane().setDefaultButton(compile_button);
		}
	} 
   
   
    /* Used by makeCompactGrid. */
    private static SpringLayout.Constraints getConstraintsForCell(
								  int row, int col,
								  Container parent,
								  int cols) {
        SpringLayout layout = (SpringLayout) parent.getLayout();
        Component c = parent.getComponent(row * cols + col);
        return layout.getConstraints(c);
    }


    class ColorPane extends JTextPane
    {
		public MXJEditCfunkCompileFrame compile_frame = null;
		public ColorPane( MXJEditCfunkCompileFrame compile_frame_)
		{
			compile_frame = compile_frame_;
			this.addKeyListener(new KeyAdapter()
	    	{
				public void keyPressed(KeyEvent evt)
				{	    
		    		int keycode = evt.getKeyCode();
		    		int mod = evt.getModifiers();
		    		if(keycode == KeyEvent.VK_W &&
		       		(mod & Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()) != 0)
					{
			    		compile_frame.setVisible(false);
			    		compile_frame.dispose();
					}
				}

	    	});
	    	
	    	this.addKeyListener(new KeyAdapter()
	    	{
	    	public void keyPressed(KeyEvent evt)
				{	    
		    		int keycode = evt.getKeyCode();
		    		int mod = evt.getModifiers();
		    		if(keycode == KeyEvent.VK_E &&
		       		(mod & Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()) != 0)
					{
						MXJEditor e = _ctx.getEditor();
						if(e.isVisible())
							e.toFront();
						else
							e.setVisible(true);	

					}
				}

	    });
		
		}
		
		public void append(Color c, String s)
		{
			append(c,s,10);
		}
		public void append(Color c, String s,int fontsize) { // better implementation--uses StyleContext
		    StyleContext sc = StyleContext.getDefaultStyleContext();
		    AttributeSet aset = sc.addAttribute(SimpleAttributeSet.EMPTY,
												StyleConstants.Foreground, c);
			aset = sc.addAttribute(aset,StyleConstants.FontFamily,"courier");
			aset = sc.addAttribute(aset,StyleConstants.FontSize,new Integer(fontsize));				
	  
		    int len = getDocument().getLength(); // same value as getText().length();
		    setCaretPosition(len);  // place caret at the end (with no selection)
		    setCharacterAttributes(aset, false);
		    replaceSelection(s); // there is no selection, so inserts at caret
			
		}

    }
}
