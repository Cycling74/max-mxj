package com.cycling74.max;
/**
 *
 * <code>MaxBox</code> represents the graphical element of an external
 * in a patcher. This can be used in conjunction with <code>MaxPatcher</code>
 * to dynamically modify and create patchers on the fly. The interface exposed is
 * very similar to the functionality exposed by the js javascript external and thus
 * much of that documentation for that external is applicable well. There is no public
 * constructor for MaxBox. MaxBoxes are created and gotten via the <code>MaxPatcher</code>
 * class.
 * <br>
 *<PRE>
 * public class maxboxtest extends MaxObject {
 *	
 *	private MaxPatcher _p = null;
 *	private Callback cb;
 *	private MaxClock cl;
 *	
 *	maxboxtest() {
 *		_p = this.getParentPatcher();
 *	
 *	}
 *	
 *	
 *  public void printboxes()
 *       {
 *           MaxBox[] boxes = _p.getAllBoxes();
 *           for(int i = 0; i < boxes.length; i++)
 *           {
 *              MaxBox b = boxes[i];
 *              post("Box "+i+": "+b.getName()+"  "+b.getMaxClass());
 *           } 
 *        }
 * }
 *   </PRE>
 *   <br>
 * 
 * created on 22-Jun-2004
 * @author Topher LaFata
 */

public class MaxBox
{
	private long       _p_box;
	private MaxPatcher _parent;

	//package visible constructor	
	protected MaxBox(MaxPatcher parent, long p_box)
	{
		_parent = parent;
		_p_box  = p_box;	
	}
	
	/**
	 * @return true if box is a bpatcher or subpatcher
	 */
	public boolean isPatcher()
	{
		if(getMaxClass().equals("jpatcher"))
			return true;
		return false;
	}
	/**
	 * Gets the patcher relative screen coordinates of the box.
	 * x1,y1 is the upper left hand corner of the box and x2,y2 is the lower
	 * right hand corner.
	 * @return int array with four elements in the folowing order: x1,y1,x2,y2
	 */
	public native int[] getRect();
	
	/**
	 * @return the name of the type of max object the box represents
	 */
	public native String getMaxClass();
	/**
	 * @return the <code>MaxPatcher</code> containing this box.
	 */
	public MaxPatcher getPatcher()
	{
		return _parent;
	}
	/**
	 * 
	 * @return true if this box is hidden on patcher lock.
	 */
	public native boolean getHidden();
	/** 
	 * Gets an integer representing the color of the box.
	 * This int is an index into the same color table visible
	 * in the Max Object menu under the Color menu. 
	 * @return the color of the box  
	 */
	public native int getColorIndex();
 	/**
	* Gets the next box after this one in the patcher. This doesn't always
	* correspond how the boxes are laid out on the screen.
	*@return the next <code>MaxBox</code> in the patchers list of boxes.
	*/
	public MaxBox getNextBox()
	{
		long p_box = 0;
		if((p_box = _get_next_box()) != 0)
			return new MaxBox(_parent,p_box);
		return null;
	}
	private native long _get_next_box();
	
	/**
	 * @return true if the box can be highlighted
	 */
	public native boolean isHighlightable();
	/**
	 * @return true if the box is part of the patcher background layer.
	 */
	public native boolean inBackground();
	/**
	 * @return true if the box is currently selected.
	 */
	public native boolean isSelected();
	/**
	 * @return true if the box is currently ignoring mouse clicks.
	 */
	public native boolean getIgnoreClick();
	/**
	 * In a patcher box names can be set via the Max application Object menu
	 * name... item. By naming a box you can have access to it via the getNamedBox
	 * method of <code> MaxPatcher</code>
	 * @return the scripting name of the box
	 */
	public native String  getName();
		
	/**
	 * Set the location and size of the box in patcher relative coordinates.
	 * @param x1 upper left x 
	 * @param y1 upper left y
	 * @param x2 lower right x
	 * @param y2 lower right y
	 */		
	public native void setRect(int x1, int x2, int y1, int y2);
	/**
	 * Set whether or not this box will be hidden when its parent
	 * patcher is locked.
	 * @param b true to hide
	 */
	public native void setHidden(boolean b);
	/**
	 * Set the color of the box as an index into the color table
	 * found in the color submenu of the Max application menu.
	 * @param color index into the color table. This value wraps
	 * so all ints are valid.
	 */
	public native void setColorIndex(int color);
	/**
	 * Set whether or not this box will be put in the background layer
	 * of its parent patcher.
	 * @param b true for background
	 */
	public native void toBackground(boolean b);
	/**
	 * Set whether or not this box responds to mouse clicks.
	 * @param b true to ignore clicks
	 */
	public native void setIgnoreClick(boolean b);
	/**
	 * Set the name of this box. This is the same as seeting the name
	 * of the box via the "name..." menu item in the Object menu of the Max 
	 * application.
	 * @param name the scripting name of the box
	 */
	public native void setName(String name);
	/**
	 * Send this box the "bang" message.
	 * @return true if the bang was sent successfully
	 */
	public native boolean bang();
	/**
	 * Send this box the "int" message.
	 * @param i the int to send to this box.
	 * @return true if the int was sent successfully
	 */
	public native boolean send(int i);
	/**
	 * Send this box the "float" message.
	 * @param f the float to send to this box.
	 * @return true if the float was sent successfully
	 */
	public native boolean send(float f);
	/**
	 * Send this box an arbitrary message.
	 * @param message the message to send to this box.
	 * @param args the arguments for the message. Can be null
	 * if the message requires no arguments.
	 * @return true if the message was sent successfully
	 */
	public native boolean send(String message, Atom[] args);
	/** 
	 * Make this box invisible in when its parent patcher is locked.
	 */
	public  void hide()
	{
		setHidden(true);
	}
	/** 
	* Keep this box visible in when its parent patcher is locked.
	*/
	public  void show()
	{
		setHidden(false);
	}
	/** 
	* Test if a particular message is understood by this box
	* @param message the message name you wish to test.
	* @return true if this box will respond to the message
	*/
	public native boolean understands(String message);
	/**
	* If the object has a floating inspector this method wil luanch it.
	*/
	public native void inspect();
	/**
	*Remove this box from its parent patcher.
	*/
	public native void remove();
	
	private native long _get_subpatcher();
	/**
	* Get the patcher contained within this box. This is only valid
	* for subpatchers and bpatchers.
	* @return the <code> MaxPatcher </code> represented by this box or
	* null if this box contains no subpatcher.
	*/
	public MaxPatcher getSubPatcher()
	{
		long p_p = 0;
		if((p_p = _get_subpatcher()) != 0)
		{
			return new MaxPatcher(p_p);
		}
		return null;
	}
	
	protected long getPeer()
	{
		return _p_box;
	}
		
	protected void finalize(){
		_free();
	}
	
	private native void _free();
	
	public boolean equals(Object o)
	{
		if(o instanceof MaxBox)
		{
			MaxBox b = (MaxBox)o;
			if(this._p_box == b._p_box)
				return true;
		}
		return false;
	}

	public int hashCode()
	{
		return (int)_p_box;
	}
		
}