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

import java.util.Timer;
import java.util.TimerTask;

/**
 * Utility class providing convenient ways to execute tasks aynchronously.
 *
 * @author Herb Jellinek
 */
public class Async {

    private static Timer sTimer = null;

    private static class Task extends TimerTask {

	private Runnable mRunnable;

	private Task(Runnable r) {
	    mRunnable = r;
	}

	public void run() {
	    mRunnable.run();
	}
    }

    /**
     * Initialize the global Timer, if necessary.
     */
    private static void init() {
	if (sTimer == null) {
	    sTimer = new Timer();
	}
    }

    /**
     * Execute a task that will complete quickly.
     * For instance, use this to perform AWT operations that would otherwise
     * lock up Max:
     * <pre>
     * myFrame = new Frame("Note display");
     * Async.quick(new Runnable() {
     *                public void run() {
     *                    myFrame.setVisible(true);
     *                }
     *             });
     * </pre>
     * @param r The code to run, as a Runnable
     */
    public static void quick(Runnable r) {
	quick(r, 0L);
    }

    /**
     * Execute a task that will start soon (in <tt>delayMSec</tt> milliseconds)
     * and complete quickly.
     * For instance, we use this to take down the splash applet after a delay.
     *
     * @param r The code to run, as a Runnable
     * @param delayMSec time until task executes, in milliseconds.
     */
    public static void quick(Runnable r, long delayMSec) {
	init();

	Task t = new Task(r);
	sTimer.schedule(t, delayMSec);
    }

    /**
     * Execute a task that might not complete quickly or at all.  This works
     * by creating and starting a new <tt>Thread</tt>.
     * For instance, use this to perform long operations that would otherwise
     * lock up Max:
     * <pre>
     * PrimeFinder pf = new PrimeFinder();
     * Async.slow(new Runnable() {
     *                public void run() {
     *                    pf.findAllPrimeNumbersUpTo(VERY_LARGE_NUMBER);
     *                }
     *             });
     * </pre>
     * @param r The code to run, as a Runnable
     */
    public static void slow(Runnable r) {
	Thread t = new Thread(r);
	t.start();
    }
}

	
