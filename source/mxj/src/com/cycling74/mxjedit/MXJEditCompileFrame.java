package com.cycling74.mxjedit;


import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.io.*;
import javax.swing.text.*;
import java.awt.Color;
import com.cycling74.max.MXJPreferences;

public class MXJEditCompileFrame extends JFrame
{
    JTextField tf_cc = null;
    JTextField tf_bd = null;
    JTextField tf_cp = null;
    JTextField tf_sf = null;
    JTextField tf_co = null;
    
    ColorPane messages = null;
    
    JButton compile_button = null;
    JButton close_button = null;
    JButton compiler_browse_button = null;
    
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
									MXJEditCompileFrame cf = cp.compile_frame;
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

    
    public MXJEditCompileFrame(MXJEditCompileContext ctx)
    {
	//top panel
	JPanel p = new JPanel(new SpringLayout());
	JLabel label_sf = new JLabel("Source File:");
	JLabel label_cc = new JLabel("Compiler Command:");
	JLabel label_bd = new JLabel("Build Directory:");
	JLabel label_cp = new JLabel("Classpath:");
	JLabel label_co = new JLabel("Compiler Options:");

	tf_sf = new JTextField(30);
	tf_sf.setText(ctx.getSourceFile());
	tf_sf.setCaretPosition(0);
	
	tf_cc = new JTextField(30);
	tf_cc.setText(ctx.getCompileCommand());
	tf_cc.setCaretPosition(0);
	
	tf_bd = new JTextField(30);
	tf_bd.setText(ctx.getBuildDirectory());
	tf_bd.setCaretPosition(0);
	
	tf_cp = new JTextField(30);
	tf_cp.setText(ctx.getClassPath());
	tf_cp.setCaretPosition(0);	
	
	tf_co = new JTextField(30);
	tf_co.setCaretPosition(0);
	_ctx = ctx;
	
	label_sf.setLabelFor(tf_sf);
	label_cc.setLabelFor(tf_cc);
	label_bd.setLabelFor(tf_bd);
	label_cp.setLabelFor(tf_cp);
	label_co.setLabelFor(tf_co);

	compiler_browse_button   = new JButton("Browse...");
	compiler_browse_button.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent e)
		{
		    compiler_browse();
		}
	   });

	
	p.add(label_sf);
	p.add(tf_sf);
	p.add(Box.createHorizontalStrut(10)); 
	p.add(label_cc);
	p.add(tf_cc);
	p.add(compiler_browse_button);
	p.add(label_bd);
	p.add(tf_bd);
	p.add(Box.createHorizontalStrut(10));
	p.add(label_cp);
	p.add(tf_cp);
	p.add(Box.createHorizontalStrut(10));
	p.add(label_co);
	p.add(tf_co);
	p.add(Box.createHorizontalStrut(10));
	p.setBorder(BorderFactory.createEtchedBorder());
	makeCompactGrid(p,
			5,3, //rows , cols
			6,6, //init x and init y
			6,6); //x pad y pad
	p.setOpaque(true);
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
	getContentPane().add(p,BorderLayout.NORTH);
	getContentPane().add(jsp,BorderLayout.CENTER);
	getContentPane().add(ap,BorderLayout.SOUTH);
	

	
	
	setTitle("MXJ Compile Window");
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
    	if(!tf_sf.getText().equals(ctx.getSourceFile()))
    	{
    		tf_sf.setText(ctx.getSourceFile());
			tf_sf.setCaretPosition(0);
		}
		if(!tf_cc.getText().equals(ctx.getCompileCommand()))
    	{
			tf_cc.setText(ctx.getCompileCommand());
			tf_cc.setCaretPosition(0);
		}
		if(!tf_bd.getText().equals(ctx.getBuildDirectory()))
    	{
			tf_bd.setText(ctx.getBuildDirectory());
			tf_bd.setCaretPosition(0);
		}
		if(!tf_cp.getText().equals(ctx.getClassPath()))
		{
			tf_cp.setText(ctx.getClassPath());
			tf_cp.setCaretPosition(0);
		}	  
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
    	
		String sourcefile = tf_sf.getText();
		String compiler  = tf_cc.getText();
		String build_dir = tf_bd.getText();
		String classpath = tf_cp.getText();
		String options   = tf_co.getText().trim();//this isn't used right now. we probably need to split these out
		
		boolean error = false;
 		messages.setEditable(true);
    	messages.setText(null);
		setSize(750,320);
		
		if(!(new File(compiler)).exists())
		{
			messages.append(ERROR_COLOR,"Error: Unable to find java compiler "+compiler);
			error = true;
		}
		else
		{
			//update the preferences so this compiler the default in the future
			if(!compiler.equals(_ctx.getCompileCommand()))
				MXJPreferences.setMXJCompilerCommmand(compiler);	
		
		}
		if(classpath.equals("") || classpath == null)
		{
			messages.append(ERROR_COLOR,"Error: Classpath must be specified.");
			error = true;
		}
		//if(build_dir.equals("") || build_dir == null )
		//{	
		//	messages.append(ERROR_COLOR,"Error: Build directory must be specified.");	
		//	error = true;
		//}
		//else if(!(new File(build_dir).exists()))
		//{
		//	messages.append(ERROR_COLOR,"Error: Build directory "+build_dir+" does not exist.");
		//	error = true;
		//}
		//else 
		//{//make build root sticky
		//	if(!build_dir.equals(_ctx.getBuildDirectory()))
		//		MXJPreferences.setMXJCompilerBuildRoot(build_dir);	
		//}
		//If someone wants to change the default quickie build directory they must do it by hand
		//in mxjpreferences.xml
			
 		if(!error)
 		{
 			
			if(_ctx.getEditor() != null)
				_ctx.getEditor().menu_save();
 			try
    		{ 
			
				messages.removeCaretListener(_error_lineno_dispatch);
				String[] opts = options.trim().split("\\s+");//split on white space;
				String[] cmd = null;
				if(opts.length == 1 && opts[0].equals(""))
				{
					cmd = new String[]{compiler/*,"-d", build_dir*/,"-classpath",classpath,
						    sourcefile};
				}
				else
				{	
					//System.out.println("options string is "+options); 
					//System.out.println("opts length is "+opts.length);
					cmd = new String[4+opts.length];
					cmd[0] = compiler;
			
					for(int i = 1; i < opts.length + 1;i++)
					{
						cmd[i] = opts[i-1];
					}

					//cmd[opts.length+1] = "-d";
					//cmd[opts.length+2] = build_dir;
					cmd[opts.length+1] = "-classpath";
					cmd[opts.length+2] = classpath;
					cmd[opts.length+3] = sourcefile;
				}

				
				Runtime rt = Runtime.getRuntime();
				Process proc = rt.exec(cmd);
					
				InputStream stderr = proc.getErrorStream();
				InputStreamReader isr = new InputStreamReader(stderr);
				BufferedReader br = new BufferedReader(isr);
				messages.append(MESSAGE_COLOR,"Compiling "+sourcefile+".........\n\n");

				//need a better implementation of priting out the command
				for(int i=0; i < cmd.length;i++)
					messages.append(MESSAGE_COLOR,cmd[i]+" ");
				messages.append(MESSAGE_COLOR,"\n\n");

				
				String line = null;
				
				String err_lineno   = null;
				String err_msg      = null;
				String err_rest_msg = null; 
				int err_count = 0;
				int idx = -1;
				
				while ( (line = br.readLine()) != null)
		    	{
		    		if((idx = line.indexOf(".java")) != -1)
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
							//tab = 8 spaces...we just force it here
							line = line.replaceAll("\t","        ");
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
   
   private void compiler_browse()
   {
   
   		SwingUtilities.invokeLater(new Runnable(){
   			public void run()
   			{
   				 FileDialog fd = new FileDialog(MXJEditCompileFrame.this);
    			fd.setVisible(true);
    			String filename = fd.getFile();//null if cancel
    			if(filename != null)
    	 		tf_cc.setText(fd.getDirectory()+filename);				
   			
   			}
   		});
   
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
		public MXJEditCompileFrame compile_frame = null;
		public ColorPane( MXJEditCompileFrame compile_frame_)
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
