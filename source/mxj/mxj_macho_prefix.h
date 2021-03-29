#pragma once
#ifndef _MXJ_MACHO_PREFIX_H_
#define _MXJ_MACHO_PREFIX_H_

#ifndef WIN_VERSION
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

#include "ext.h"
#include "ext_wind.h"
#include "ext_user.h"
#include "ext_sysmem.h"
#include "ext_path.h"

#ifdef WIN_VERSION
	#define MXJ_JNI_CREATE_JAVA_VM	g_ifn.CreateJavaVM
#else
	#define MXJ_JNI_CREATE_JAVA_VM	JNI_CreateJavaVM
#endif

#define MXJ_JNI_CALL(e,m) 		(*e)->m
#define MXJ_INVOKE_CALL(vm,m)	(*vm)->m

#endif // _MXJ_MACHO_PREFIX_H_
