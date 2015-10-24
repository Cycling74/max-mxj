//
//  JavaHomeOsx.h
//  mxj
//  Copyright (c) 2015 Peter Slack ,Maplepost. All rights reserved.
//  Created by Peter Slack on 2015-10-17.
//
//

#ifndef __mxj__JavaHomeOsx__
#define __mxj__JavaHomeOsx__

#define _tcsicmp strcasecmp
#define _TCHAR char
#define _T_ECLIPSE(s) s
#define _tcsstr strstr
#define _tcsrchr strrchr


char * getJavaVersion(char* command);
char * getJavaHome();
char * findVMLibrary( char* command );
char * findLib( char* command );


#endif /* defined(__mxj__JavaHomeOsx__) */
