#include "mxj_common.h"
#include "mxj_methodlist.h"
#include "mxj_utils.h"

t_mxj_methodlist *mxj_methodlist_new()
{
	t_mxj_methodlist* ml;
	
	ml = (t_mxj_methodlist*) sysmem_newptr(sizeof(t_mxj_methodlist));
	ml->mlist = NULL;
	ml->size  = 0;
	
	return ml;
}

void mxj_methodlist_add(t_mxj_methodlist *methodlist,t_mxj_method *m)
{
	//do all your pointer checks on method list etc.
	int i;
	
	C74_ASSERT(methodlist)

	for (i=0;i< methodlist->size;i++) {
		if (m->name == methodlist->mlist[i]->name) {
			mxj_method_add_sig(methodlist->mlist[i],m);
			
			return;
		}
	}
	
	//first instance of a methodname, so we need to allocate/ realloc our method struct
	methodlist->size++;
	
	if (methodlist->size > 1)
		methodlist->mlist = (t_mxj_method**)sysmem_resizeptr((void *)methodlist->mlist,methodlist->size*sizeof(t_mxj_method*));
	else
		methodlist->mlist = (t_mxj_method**)sysmem_newptr(methodlist->size*sizeof(t_mxj_method*));
	
	methodlist->mlist[methodlist->size - 1] = m;
	
	return;
}


void mxj_methodlist_free(t_mxj_methodlist *ml)
{
	int i;
	
	C74_ASSERT(ml)

	for(i = 0; i < ml->size; i++)
		mxj_method_free(ml->mlist[i]);
	
	sysmem_freeptr(ml);
}

int mxj_methodlist_methodexists(t_mxj_methodlist *ml,t_symbol *name, t_symbol *java_p_types)
{
	int i,p;
	t_mxj_method *m;
	
	C74_ASSERT(ml)

	for(i = 0; i < ml->size; i++) {
		m = ml->mlist[i];
		
		if(m->name == name) {
			for(p = 0; i < m->sigcount;p++) {
				if(	m->jp_types[i] == java_p_types)
					return 1;
			}
		}
	}
	
	return 0;
}

t_mxj_method *mxj_methodlist_resolve(t_mxj_methodlist *ml,char *name, long p_argc,
									 char* param_types, int *resolve_status, int *offset)
{
	int i;
	t_mxj_method *m;
	*resolve_status    = -1;
	*offset            = 0;
	
	C74_ASSERT(ml)
	
	for(i = 0; i < ml->size; i++) {
		if(!strcmp(ml->mlist[i]->name->s_name, name)) {	// we matched the name 
			m = ml->mlist[i];
			for(i = 0; i < m->sigcount;i++) {	// try and match the parameter types
				//GIMME OVERRIDES ALL
				if(m->p_types[i]->s_name[0] == 'G') {
					*offset         = i;
					*resolve_status = MXJ_GIMME_MATCH;

					return m;
				}
				
				//ARRAY OVERRIDES ALL WHEN FIRST ARG IS THE SAME TYPE AS THE ARRAY
				if(m->p_types[i]->s_name[0] == '[') {
					if(param_types[0] == m->p_types[i]->s_name[1] )
					{
						*offset = i;
						*resolve_status = MXJ_ARRAY_MATCH;
						continue;	// need to make sure there is no GIMME
					}
				}
				
				if(!strcmp(m->p_types[i]->s_name,param_types)) {
					if(*resolve_status != MXJ_ARRAY_MATCH) {
						*offset         = i;
						*resolve_status = MXJ_METHOD_MATCH;
					}
					continue;	// we need to make sure there are no [ or GIMMIES to override the match
				}
				
				// this is so we end up coercing to the closest match
				// we try to match on arg length. If there is argc match
				// then the first method named name gets called.
				
				if(p_argc == m->p_argc[i] && *resolve_status == -1)
					*offset = i;
			}
			
			if(*resolve_status != MXJ_METHOD_MATCH)
				*resolve_status = MXJ_COERCE;
			
			return m;
		}
	}
	
	*resolve_status = MXJ_NO_METHOD_MATCH;
	
	return NULL;	// no method with name was found. call anything.
}
