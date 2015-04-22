#ifndef __MXJ_UTILS
#define __MXJ_UTILS

char* mxj_getbytes(t_atom_long size);
void mxj_freebytes(void *vp, t_atom_long size);
short max_path_to_native_path(char *max_path_in,char *native_path_out);

// since jlong is defined as long long in Java we need this conversion to silence warnings in 32 bit
#define p_to_jlong(x) (jlong)(t_atom_long)x

#endif
