package com.cycling74.max;

import java.util.*;

/**
 * The context and company in which a Max object finds itself.
 * @see MaxObject
 *
 * @author Herb Jellinek
 */
public final class MaxContext {

    private static final String MXJ_VERSION = "mxj 1.0 for Java 1.4";

    /**
     * The singleton context.
     */
    private static final MaxContext sSingleton = new MaxContext();

    /**
     * All registered MaxObject instances.
     */
    private final Map mAllInstances = new HashMap(10);
	
    /**
     * Create one.
     */
    private MaxContext() {
    }

    /**
     * Register an instance.
     */
    static void register(Object o) {
		synchronized (sSingleton) {
		    sSingleton.mAllInstances.put(o,o);
		}
    }

    /**
     * Unregister an instance.
     */
    static void unregister(Object o) {
		synchronized (sSingleton) {
		    sSingleton.mAllInstances.remove(o);
		}
    }

    /**
     * Return the singleton instance.
     */
    static MaxContext getSingleton() {
    	return sSingleton;
    }
    

	
    /**
     * Find an instance with a given name.  An object can set its own
     * name using <tt>MaxObject.setName</tt>.
     * @param name the given name
     * @return the MaxObject with the given name, or null if it doesn't exist
     * @see com.cycling74.max.MaxObject#setName(String)
     */
    public MaxObject getMaxObject(String name) {
		if (name == null) {
	    	return null;
		}
		synchronized (this) 
		{
	    	Iterator i = mAllInstances.keySet().iterator();
	    	while (i.hasNext())
	    	{
				Object o = i.next();
				if(o instanceof MaxObject)
				{
					if (name.equals(((MaxObject)o).getName())) 
					{
		    			return (MaxObject)o;
					}
	    		}
			}
			return null;
    	}
    }

    /**
     * Return a <tt>Set</tt> containing all current MaxObject instances.
     * If there are none, return the empty <tt>Set</tt>.
     * The returned <tt>Set</tt> is not modifiable.
     * @return a <tt>Set</tt> containing all current MaxObject instances
     */
    public Set getAllObjects() {
		synchronized (this) {
		    return Collections.unmodifiableSet(mAllInstances.entrySet());
		}
    }

    /**
     * Return the mxj version information.  
     * @return a <tt>String</tt> describing this version of mxj
     */
    public String getMxjVersion() {
    	return MXJ_VERSION;
    }
}
