/*
 * Copyright (c) 2003, Cycling '74
 * 
 * This software is the confidential and proprietary information of 
 * Cycling '74 ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Cycling '74.
 * 
 * CYCLING '74 MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
 * CYCLING '74 SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 *
 */

package com.cycling74.util;

import java.net.*;
import java.awt.*;
import java.awt.image.*;

/**
 * Image and graphics utilities, some of which were adapted from the
 * Utilities class included in Jonathan Knudsen's book
 * <i>Java 2D Graphics</i> (O'Reilly).
 * 
 * @author Herb Jellinek
 */
public class ImageUtilities {

    private static int sID = 0;

    /**
     * MediaTracker demands a component, so we use an instance of an anonymous
     * inner class to satisfy it.
     */
    private static final Component sComponent = new Component() { };

    private static final MediaTracker sTracker = new MediaTracker(sComponent);

    /**
     * You can't instantiate this class.
     */
    private ImageUtilities() {
    }

    /**
     * Start loading the image and wait for it to complete.
     */
    public static boolean waitForImage(Image image) {
	int id;
	synchronized (sComponent) {
	    id = sID++;
	}
	sTracker.addImage(image, id);
	try {
	    sTracker.waitForID(id);
	} catch (InterruptedException ie) {
	    return false;
	}
	if (sTracker.isErrorID(id)) {
	    return false;
	}
	return true;
    }

    /**
     * Buffer an image in memory in RGB format.
     */
    public static BufferedImage makeBufferedImage(Image image) {
	return makeBufferedImage(image, BufferedImage.TYPE_INT_RGB);
    }
    
    /**
     * Buffer an image in memory in some given format.
     */
    public static BufferedImage makeBufferedImage(Image image, int imageType) {
	if (!waitForImage(image)) {
	    return null;
	}
	
	BufferedImage bufferedImage =
	    new BufferedImage(image.getWidth(null), image.getHeight(null),
			      imageType);
	Graphics2D g2 = bufferedImage.createGraphics();
	g2.drawImage(image, null, null);
	return bufferedImage;
    }

    /**
     * Load the image at the URL.  Returns the image, or null if it
     * could not be loaded.
     */
    public static Image blockingLoad(URL url) {
	Image image = Toolkit.getDefaultToolkit().getImage(url);
	if (!waitForImage(image)) {
	    return null;
	}
	return image;
    }

    /**
     * Show or hide a <tt>Component</tt> (<tt>Frame</tt>,
     * <tt>JFrame</tt>, etc.) asynchronously.
     */
    public static void setVisible(final Component comp, final boolean vis) {
	Async.quick(new Runnable() {
		public void run() {
		    comp.setVisible(vis);
		}
	    });
    }
}
