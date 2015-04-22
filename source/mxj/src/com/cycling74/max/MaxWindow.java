package com.cycling74.max;
/**
 *
 * <code>MaxWindow</code> is a reference to the window containing a patcher.
 * This can be used in conjunction with <code>MaxPatcher</code> and <code>MaxBox</code>
 * to dynamically modify and create patchers on the fly. The interface exposed is
 * very similar to the functionality exposed by the js javascript external and thus
 * much of that documentation for that external is applicable well. There is no public
 * constructor for MaxWindow. MaxWindows created / gotten via the <code>MaxPatcher</code>
 * class.
 *
 * created on 22-Jun-2004
 * @author Topher LaFata
 */
public class MaxWindow
{
	private long _p_wind;
	private MaxPatcher _patcher = null;
	
	//package visible constructor
	protected MaxWindow(long p_wind)
	{
		_p_wind = p_wind;
		_patcher = null;
	}
	/**
	*@return true if window is visible on the screen
	*/
	public native boolean isVisible();
	/**
	*Set whether or not the window is visible
	*/
	public native void setVisible(boolean b); 
	/**
	* Get the title of the window as displayed in the title bar.
	* If there are any brackets, as in the case of subpatchers,
	* they are removed.
	*/
	public native String getTitle();
	/**
	* Set the title of the window in the title bar.
	*/
	public native void setTitle(String title);
	/**
	*@return true if the window has a horizontal scrollbar
	*/
	public native boolean hasHorizontalScroll();
	/**
	*@return true if the window has a vertical scrollbar
	*/
	public native boolean hasVerticalScroll();
	/**
	*@return true if the window has a zoom widget
	*/
	public native boolean hasZoom();
	/**
	*@return true if the window has a title bar
	*/
	public native boolean hasTitleBar();
	/**
	*@return true if the window has a close widget
	*/
	public native boolean hasClose();
	/**
	*@return true if the window has a resize widget
	*/
	public native boolean hasGrow();
	/**
	*@return 2 element int array containing width and height of the window
	*/
	public native int[] getSize();
	/**
	*Set the size of the window
	*/
	public native void setSize(int width, int height);
	/**
	*@return 4 element array containing absolute screen coordinates of the
	* window's bounding box  x1,y1,x2,y2
	*/
	public native int[] getLocation();
	/**
	*Set the bounding rect of the window.
	*/
	public native void setLocation(int x1, int y1, int x2, int y2);
	private native long _get_patcher();
	/**
	*@return the <code>MaxPatcher</code> object for which this window is a host.
	*/
	public MaxPatcher getPatcher()
	{
		if(_patcher != null)
			return _patcher;
		else	
		{
			long p_p = _get_patcher();
			if(p_p != 0)
			{
				_patcher = new MaxPatcher(p_p);
				return _patcher;
			}
			else
				return null;
		}	
	
	}
	/**
	*@return the max class of the patcher for which this window is a host.
	*/
	public native String getPatcherClass();
	/**
	*@return whether or not this window or its patcher have changed since opened.
	*/
	public native boolean isDirty();
	/**
	*Set whether or not this window and its patcher should be treated as dirty.
	* When a window is dirty the user is asked if they wish to save changes when it is
	*closed.
	*/
	public native void setDirty(boolean isdirty);
	/**
	*Set whether or not the window has a zoom widget
	*/
	public native void setZoom(boolean haszoom);
		/**
	*Set whether or not the window has a titlebar
	*/
	public native void setTitleBar(boolean hastitlebar);
	/**
	*Set whether or not the window has a close widget
	*/
	public native void setClose(boolean hasclose);
	/**
	*Set whether or not the window has a grow widget
	*/
	public native void setGrow(boolean hasgrow);
	/**
	*Set whether or not the window is "floating"
	*/
	public native void setFloat(boolean floating);
	/**
	* close the window. USE AT PERIL.
	*/
	public void close()
	{
		_close();
		_p_wind = 0;
	}
	private native void _close();

	public boolean equals(Object o)
	{
		if(o instanceof MaxWindow)
		{
			MaxWindow w = (MaxWindow)o;
			if(this._p_wind == w._p_wind)
				return true;
		}
		return false;
	}

	public int hashCode()
	{
		return (int)_p_wind;
	}
}