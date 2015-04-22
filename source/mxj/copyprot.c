/**
 * Support for copy protection.
 */

#include "copyprot.h"
#include "ext_obex.h"

// Have we already told the user that we're not authorized to run?
static t_bool g_warnedUser = FALSE;

long isMaxRuntime(void);


/*
 * This determines if you're in the runtime version of Max by creating a patcher and seeing
 * if it can be dirtied.
 * This needs to be in the mxj code so it can't be spoofed by a trojan-horsed kernel.
 * It returns true if you're in runtime, false if not.
 * (David Zicarelli wrote this.)
 */
static long isMaxRuntime(void) {
    t_patcher *p;
    t_symbol *nsym, *psym;
    t_messlist *m;
    void *pcache, *bcache;
    long rv=0;

    // determine if in runtime by instantiating a new vpatcher,
    // calling its dirty method and seeing if it really gets dirty.
    psym = gensym("vpatcher");
    nsym = gensym("#N");
       
    if (nsym->s_thing) {
        m=ob_messlist(nsym->s_thing);
        while (m->m_sym) {
            if (m->m_sym==psym) {
                pcache=gensym("#P")->s_thing;
                bcache=gensym("#B")->s_thing;
                if (p = (t_patcher *)(*(m->m_fun))(NULL)) {
                    typedmess((t_object *)p,gensym("dirty"),0,0L);
					// updating for Max 5 (no direct struct access) -- tap
                    //if (!(p->p_wind->w_dirty)) rv=1;
					if (!object_attr_getlong(p, _sym_dirty)) rv=1;
                    typedmess((t_object *)p,gensym("clean"),0,0L);
                    freeobject((t_object *)p);
                }
                gensym("#P")->s_thing = (t_object *)pcache;
                gensym("#B")->s_thing = (t_object *)bcache;
                return rv;
            }
            m++;
        }
    }
    return rv;
}

/**
 * It's OK to run mxj if one of these is true:
 * - We're running in the Max runtime.
 * - It's still the 30-day Max demo period
 * - This is an authorized copy of MXJ.
 */
t_bool okToRun() 
{
	return TRUE; //for the time being always allow	
/*
	if (isMaxRuntime()) {
		return TRUE;
	} else {
		long n = 0x8523;
		method f = (method)gensym("&h5r$$3*vc@e")->s_thing;
		if (f) {
		    f(&n);
		}
		
		if (n == 1) {
		    // you are authorized as part of the 30-day Max demo period
		    return TRUE;
	    } else if (n == 0) {
		    // you are authorized by an explicit MXJ authorization
		    return TRUE;
		} else {
			// you are not authorized
			return FALSE;
		}
	}
*/
}

/*
 * Warn the user once that we're not authorized to run.
 */
void notAuthorizedMessage(void) {
	if (! g_warnedUser) {
		post("");
		post("* * * * * * * * * * * * * * * * * *");
		post("");
		post("Sorry, mxj is not authorized on this computer.");
		post("If you would like to purchase an mxj authorization,");
		post("please visit the Cycling '74 website, http://www.cycling74.com.");
		post("");
		post("* * * * * * * * * * * * * * * * * *");
		post("");

		g_warnedUser = TRUE;
	}
}


