#ifndef _MXJ_METHODLIST_H_
#define _MXJ_METHODLIST_H_

#include "mxj_method.h"

typedef struct _mxj_methodlist
{
	long 			size;
	t_mxj_method	**mlist;
} t_mxj_methodlist;

t_mxj_methodlist *mxj_methodlist_new(void);

void mxj_methodlist_add(t_mxj_methodlist *x, t_mxj_method *m);
int mxj_methodlist_methodexists(t_mxj_methodlist *ml,t_symbol *name, t_symbol *java_param_types);
t_mxj_method *mxj_methodlist_resolve(t_mxj_methodlist *x,char *name, long p_argc,char* param_types, int *resolve_status, int *offset);
void mxj_methodlist_free(t_mxj_methodlist *x);

//resolve status for methodlist_resolve
#define MXJ_METHOD_MATCH	10    //method found that matches paramters
#define MXJ_GIMME_MATCH		11    //no exact match but GIMME method with same name
#define MXJ_COERCE			12    // no gimme method but name method exisits with different param types
#define MXJ_NO_METHOD_MATCH	13 //no method of name found
#define MXJ_ARRAY_MATCH		14

#endif // _MXJ_METHODLIST_H_
