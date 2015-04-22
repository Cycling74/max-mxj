package com.cycling74.max;
/**
 *
 * <code>MaxPatcher</code> can be used in conjunction with <code>MaxBox</code>
 * to dynamically modify and create patchers on the fly. The interface exposed is
 * very similar to the functionality exposed by the js javascript external and thus
 * much of that documentation for that external is applicable well. 
 * <br>
 *<PRE>
 * public class maxpatchertest extends MaxObject {
 *	      
 *       public void makepatcher()
 *       {
 *
 *               MaxPatcher p  = new MaxPatcher(50,50,200,200);
 *               MaxBox b11 = p.newDefault(20,20,"toggle",null);
 *               MaxBox b21 = p.newDefault(50,20,"toggle",null);
 *               MaxBox b31 = p.newDefault(80,20,"toggle",null);
 *				 p.getWindow().setVisible(true);
 *               p.getWindow().setTitle("TEST PATCH");
 *              
 *       }
 *        
 * }
 *   </PRE>
 *   <br>
 * 
 * created on 22-Jun-2004
 * @author Topher LaFata
 */
public class MaxPatcher
{
	private long _p_patcher = 0;
	private MaxWindow _window = null;
	
	//package visible constructor
	protected MaxPatcher(long p_patcher)
	{
		_p_patcher = p_patcher;
		_window = null;
	}
	
	/**
	 * creates a <code>MaxPatcher</code>. The patcher is created
	 * in a non visible state. To see the patcher use the setVisible
	 * method on its MaxWindow.
	 * @param x1 upper left corner x in absolute screen coordinates. 
	 * @param y1  upper left corner y in absolute screen coordinates.
	 * @param x2  lower right corner x in absolute screen coordinates.
	 * @param y2 lower right corner y in absolute screen coordinates.
	 *
	 */
	public MaxPatcher(int x1, int y1, int x2, int y2)
	{
		long p = _patcher_construct(x1,y1,x2,y2);
		if(p != 0)
			_p_patcher = p;
		else
			_p_patcher = 0;	
	
		_window = null;
	}
	/**
	 * get the absolute filesystem path of the file defining the 
	 * patcher.
	 * @return the absolute filesystem path of the file defining the 
	 * patcher
	 */
	public native String getPath();
	/**
	* send an arbitrary message to the patcher. See the help file
	* for the max external thispatcher to see the sorts of messages you
	* might want to send.
	* @param msg the message to send thispatcher
	* @param args arguments for the message to thispatcher. can be null.
	*/
	public native long send(String msg, Atom[] args);	
	/**
	*set the background color.
	*@param r red component
	*@param g green component
	*@param b blue component
	*/
	public native void setBackgroundColor(int r, int g, int b);
	/**
	*@return true if this patcher is a bpatcher.
	*/
	public native boolean isBPatcher();
		
	private native long _get_named_box(String name); 
	
	/**
	*get the <code>MaxBox</code> contained in this patcher by name.
	*The name of max boxes are set via the Object menu of the max application
	* or programatically using the <code>MaxBox</code>class.
	* @param name the scripting name of the box you want a reference to
	* @return a <code>MaxBox</code> instance representing the named box
	* or null if no box named name exists in the patcher.
	*/
	public MaxBox getNamedBox(String name)
	{
		long p_box = _get_named_box(name);
		if(p_box != 0)
			return new MaxBox(this,p_box);
		else
			return null;
	}	
	/**
	*get a list of all boxes contained in this patcher. Note that this
	* will not recurse into subpatches but you can do that yourself by
	* testing if a particular box is a subpatcher via the <code>MaxBox</code>
	* isPatcher and getting a reference to it's <code>MaxPatcher</code>
	* via it's getSubPatcher() method.
	* @return array containing all the <code>MaxBox</code> instances contained
	* within this patch.
	*/
	public MaxBox[] getAllBoxes()
	{
		long[] peers = _get_all_boxes();
		
		if(peers != null)
		{
			int len = peers.length;
			MaxBox[] ret = new MaxBox[len];
			for(int i = 0; i < peers.length;i++)
				ret[i] = new MaxBox(this,peers[i]);
		
			return ret;
		}
		return null;
	}
	/**
	* Create a new <code>MaxBox</code> in this patcher.
	* Arguments can be passed as well. The example bellow shows how to create a non-UI object.
	*
	*<pre>
	*	MaxPatcher p = getParentPatcher();
	*	MaxBox b = p.newDefault(80,20,"newobj",Atom.parse("@text \"makenote 127 4n\" @textcolor 0. 0. 1. 1."));
	*</pre>
	*
	* To create a UI object:
	*<pre>
	*	MaxPatcher p = getParentPatcher();
	*	MaxBox b = p.newDefault(100,50,"button",Atom.parse("@bgcolor 0. 0. 1. 1. @fgcolor 1. 0. 0. 0.8"));
	*</pre>
	*
	* @param x location of x in patcher relative coordinates.
	* @param y location of y in patcher relative coordinates.
	* @param maxclassname the name of the max class you which to create an instance of in the patcher.
	* @param args arguments to use when instantiating the max class.
	* @return a new MaxBox or null on failure 
	*/
	public MaxBox newDefault(int x, int y, String maxclassname, Atom[] args)//args can be null
	{
		long p = 0;
		if((p = _new_default(x,y,maxclassname,args)) != 0)
		{
			return new MaxBox(this,p);
		}
		return null;
	}
	
	/**
	* Create a new <code>MaxBox</code> in this patcher. Using raw
	* max patcher file syntax. For Example:
	*<pre>
	*	--Create an instance of bpatcher containing noise~.help in patcher p.
	*
	*     p.newObject("bpatcher",Atom.parse("10 10 100 100 0 0 noise~.help 0"));
	*</pre>
	* @param maxclassname the name of the max class you which to create an instance of in the patcher.
	* @param args arguments to use when instantiating the max class.
	* @return a new MaxBox or null on failure
	*/
	public MaxBox newObject(String msg, Atom[] args)
	{
		long p = 0;
		if((p = _new_object(msg,args)) != 0)
		{
			return new MaxBox(this,p);
		}
		return null;
	}
	
	/**
	 * Connect a patch line from box b1 to box b2.
	 *@param b1 MaxBox source
	 *@param outlet outlet of the box b1 from which to originate the connection
	 *@param b2 MaxBox dest
	 *@param inlet inlet of the box b2 to which the connection is terminated
	 *@param color int representing the color index of the patchline.
	 */
	public void connect(MaxBox b1, int outlet, MaxBox b2, int inlet, int color)
	{
		_connect(b1.getPeer(), outlet, b2.getPeer(), inlet, color);
	}
	/**
	 * Connect a patch line from box b1 to box b2 with default color.
	 *@param b1 MaxBox source
	 *@param outlet outlet of the box b1 from which to originate the connection
	 *@param b2 MaxBox dest
	 *@param inlet inlet of the box b2 to which the connection is terminated
	 */
	public void connect(MaxBox b1, int outlet, MaxBox b2, int inlet)
	{
		_connect(b1.getPeer(), outlet, b2.getPeer(), inlet, 0);
	}
	/**
	*Disconnect a patch line from box b1 to box b2.
	*@param b1 MaxBox source
	*@param outlet outlet of the box b1 from which the connection originates
	*@param b2 MaxBox dest
	*@param inlet inlet of the box b2 to which the connection terminates
	*/
	public void disconnect(MaxBox b1, int outlet, MaxBox b2, int inlet)
	{
		_disconnect(b1.getPeer(), outlet, b2.getPeer(), inlet);
	}
	
	/**
	*@return the top level window containing this patcher.
	*/
	public MaxWindow getWindow()
	{	
		if(_window != null)
			return _window;
		else
		{
			long pw = _get_window_ptr();
			if(pw != 0)
			{
				_window = new MaxWindow(pw);
				return _window;
			}
			else
				return null;
		}
	}

	
	
	/**
	 * @return the name of the patcher.
	 */
	public native String getName();
	/**
	*@return true if this patcher is currently locked.
	*/
	public native boolean isLocked();
	/**
	*@return the name of the parent max class for this patcher.
	*/
	public native String getParentMaxClass();
	/**
	*@return 2 element int array containing the offset of the current display
	* of this patcher.
	*/
	public native int[] getOffset();


	/**
	 * @return two element int array representing the origin of the patcher
	 */
	public native int[] getOrigin();
	/**
	 * @return the number of boxes contained within this patcher.
	 */
	public native int getCount();
	/**
	 * @return the file path of the file containing this patcher as a max style path. 
	 */
	public native String getFilePath();
	/**
	 * Set whether or not this patcher is locked.
	 * @param b true to lock
	 */
	public native void setLocked(boolean b);
	
	private native long _new_object(String msg, Atom[] args);
	private native long _new_default(int x, int y, String maxclassname, Atom[] args);//args can be null
	private native void _connect(long b1, int out, long b2, int in, int color);
	private native void _disconnect(long b1, int out, long b2, int in);
	private native long[] _get_all_boxes();	
	private native long _patcher_construct(int x1, int y1, int x2, int y2);
	private native long _get_window_ptr();

	protected void finalize(){
		_free();
	
	}
	private native void _free();

	public boolean equals(Object o)
	{
		if(o instanceof MaxPatcher)
		{
			MaxPatcher p = (MaxPatcher)o;
			if(this._p_patcher == p._p_patcher)
				return true;
		}
		return false;
	}

	public int hashCode()
	{
		return (int)_p_patcher;
	}
	
}