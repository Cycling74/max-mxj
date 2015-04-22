

#ifndef _MXJ_PREFIX_H_

#define _MXJ_PREFIX_H_



//This file is included before compiling all the files in this project



#ifndef WIN_VERSION

#include <MacHeadersCarbon.h>
#include "machoops.h"
#endif



#ifdef MXJ_CFM_DYN_JNI

	#define MXJ_JNI_CREATE_JAVA_VM 	CFM_JNI_CreateJavaVM
	#define MXJ_JNI_CALL(e,m) 		(*(g_cfm_env.m))
	#define MXJ_INVOKE_CALL(v,m)		(*(g_cfm_jvm.m))
#else

	#ifdef WIN_VERSION
		#define MXJ_JNI_CREATE_JAVA_VM	g_ifn.CreateJavaVM
	#else
		#define MXJ_JNI_CREATE_JAVA_VM	JNI_CreateJavaVM
#endif

#define MXJ_JNI_CALL(e,m) 		(*e)->m
#define MXJ_INVOKE_CALL(vm,m)	(*vm)->m

#endif

#include "ext.h"
#include "ext_wind.h"
#include "ext_user.h"
#include "ext_sysmem.h"
#include "ext_path.h"

#endif /*_MXJ_PREFIX_H_*/

