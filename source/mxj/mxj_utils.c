#include "mxj_common.h"
#include "mxj_utils.h"

#define MAX_GETBYTES 32767

char* mxj_getbytes(t_atom_long size)
{
	if (size <= MAX_GETBYTES)
		return getbytes((short)size);
	else
		return sysmem_newptr((long)size);	

}

void mxj_freebytes(void *vp, t_atom_long size)
{
	if (size <= MAX_GETBYTES)
		freebytes(vp, (short)size);
	else
		sysmem_freeptr(vp);
}

short max_path_to_native_path(char *max_path_in,char *native_path_out)
{
#ifdef WIN_VERSION
	short ret; 
	ret = path_nameconform(max_path_in,native_path_out,PATH_STYLE_NATIVE_WIN,PATH_TYPE_ABSOLUTE); 
	return ret;
#else	// MAC_VERSION
	
	static char bootname[MAX_PATH_CHARS];
	static int boot_gen    = 0;
	static int bootnamelen = 0;
	
	int i = 0;
	char *ps;
	char volname[MAX_PATH_CHARS];
	char tmp_out[MAX_PATH_CHARS];
	
	if (!boot_gen) {
		char *p;
		
		path_nameconform("/",bootname,PATH_STYLE_MAX,PATH_TYPE_ABSOLUTE);
		p = &bootname[0];
		while(*p != ':') {
			p++;
			bootnamelen++;
		}
		
		*p = '\0';	
		boot_gen = 1;
	}
	path_nameconform(max_path_in,tmp_out,PATH_STYLE_MAX,PATH_TYPE_ABSOLUTE);
	ps = &tmp_out[0];
	while(*ps != ':')
	{
		volname[i] = *ps;
		ps++;
		i++;
	}
	
	volname[i] = '\0';
	if (!strcmp(volname,bootname))
		strcpy(native_path_out,(ps+1));	//+1 for colon	
	else
	{	
		char tmp[MAX_PATH_CHARS];
		strcpy(tmp,ps+1);
		sprintf(native_path_out,"/Volumes/%s%s",volname,tmp);
	}
	
	return 0;
#endif	// WIN_VERSION
}
