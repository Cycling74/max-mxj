#ifndef __MXJ_PROPS__
#define __MXJ_PROPS__

#define MXJPROP_DYN_CLASS_DIR			1
#define MXJPROP_LOAD_CLASS_FROM_DISK	2
#define MXJPROP_MSP_MODE				3

typedef struct mxj_prop
{
	int id;
	void *prop;
}t_mxj_prop;

typedef struct mxj_proplist
{
	int size;	// realsize
	int len;	// number of props in here
	t_mxj_prop **pptr;
} t_mxj_proplist;

t_mxj_proplist* mxj_proplist_new(int initialsize);
void mxj_proplist_add_prop(t_mxj_proplist* pl,int id, void *data);
void mxj_proplist_free(t_mxj_proplist *x);
t_mxj_prop *mxj_prop_new(int id, void *prop);
void mxj_prop_free(t_mxj_prop *x);

#endif	// __MXJ_PROPS__
