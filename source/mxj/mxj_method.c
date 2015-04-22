#include "mxj_method.h"


t_mxj_method *mxj_new_method( char *name,long p_argc, char *p_types,char * jp_types, jmethodID mid)
{
	t_mxj_method *m;

	m               = (t_mxj_method*) sysmem_newptr(sizeof(t_mxj_method));
	m->name         = gensym(name);
	m->p_argc       = (long*)sysmem_newptr(sizeof(long));           //arg count for method
	m->p_types      = (t_symbol**)sysmem_newptr(sizeof(t_symbol*)); //max land param types
	m->jp_types     = (t_symbol**)sysmem_newptr(sizeof(t_symbol*));	//java param types	
	m->mids         = (jmethodID*)sysmem_newptr(sizeof(jmethodID));
	//check pointers
	m->p_argc[0]    = p_argc;
	m->p_types[0]   = gensym(p_types);
	m->jp_types[0]  = gensym(jp_types);
	m->mids[0]      = mid;
	m->sigcount     = 1;
	
	return m;
}

//this only is garunteed to work when src only has one method
void mxj_method_add_sig(t_mxj_method *dest, t_mxj_method *src)
{
	dest->sigcount++;
	dest->p_argc       = (long*)sysmem_resizeptr((void*) dest->p_argc,dest->sigcount*sizeof(long));
	dest->p_types  =(t_symbol**)sysmem_resizeptr((void *)dest->p_types, dest->sigcount*sizeof(t_symbol*));
	dest->jp_types  =(t_symbol**)sysmem_resizeptr((void *)dest->jp_types, dest->sigcount*sizeof(t_symbol*));
	dest->mids     =(jmethodID*)sysmem_resizeptr((void *)dest->mids, dest->sigcount*sizeof(jmethodID));
	
	//this could fail check pointer here
	dest->p_argc[dest->sigcount - 1]    = src->p_argc[0];
	dest->p_types[dest->sigcount - 1]   = src->p_types[0];
	dest->jp_types[dest->sigcount - 1]  = src->jp_types[0];
	dest->mids[dest->sigcount - 1]      = src->mids[0];
	
	return;
}

void mxj_method_free(t_mxj_method *m)
{	
	sysmem_freeptr((void *)m->p_argc);
	sysmem_freeptr((void *)m->p_types);
	sysmem_freeptr((void *)m->jp_types);
	sysmem_freeptr((void *)m->mids);
	sysmem_freeptr((void *)m);
}
