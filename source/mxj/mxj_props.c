#include "ext_sysmem.h"
#include "mxj_props.h"

t_mxj_proplist *mxj_proplist_new(int initialsize)
{
	t_mxj_proplist *pl = (t_mxj_proplist *)sysmem_newptr(sizeof(t_mxj_proplist));
	pl->size = initialsize;
	pl->pptr = (void *)sysmem_newptr(initialsize * sizeof(t_mxj_prop));
	pl->len = 0;
	
	return pl;
}

void mxj_proplist_add_prop(t_mxj_proplist* pl,int id, void *data)
{
	if (pl != NULL) {
		if (pl->len == pl->size) {
			pl->size = pl->size + (8 * sizeof(t_mxj_prop));
			sysmem_resizeptr(pl->pptr,pl->size);
		}
		
		pl->pptr[pl->len] = mxj_prop_new(id,data);
		pl->len++;	
	}
	else {
		error("proplist is uninitialized");
	}
}


void mxj_proplist_free(t_mxj_proplist *x)
{
	int i;
	for(i = 0; i < x->size;i++)
		mxj_prop_free(x->pptr[i]);

	sysmem_freeptr(x);
}


t_mxj_prop *mxj_prop_new(int id, void *prop)
{
	t_mxj_prop *p = (t_mxj_prop*)sysmem_newptr(sizeof(t_mxj_prop));
	p->id   = id;
	p->prop = prop;
	
	return p;
}


void mxj_prop_free(t_mxj_prop *x)
{
	sysmem_freeptr(x->prop);
	sysmem_freeptr(x);
}
