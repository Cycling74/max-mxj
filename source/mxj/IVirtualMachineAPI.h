//
//  IVirtualMachineAPI.h
//  mxj
//
//  Created by Peter Slack on 2015-10-17.
//
//

#ifndef mxj_IVirtualMachineAPI_h
#define mxj_IVirtualMachineAPI_h

// this is a c to c++ firewall API construct allowing us to call c++ from MAX native object

#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct ivirtualmachine ivirtualmachine;
    
    ivirtualmachine* new_virtualmachine(void);
    void delete_virtualmachine(ivirtualmachine* v);
    void start_java(ivirtualmachine* v);
    void add_java_option(ivirtualmachine* v,char * option);
    void add_java_classpath(ivirtualmachine* v,char * classpath);
    JavaVM * get_java_vm(ivirtualmachine* v);
    JNIEnv * get_thread_env(ivirtualmachine* v);
    jstring get_system_property(ivirtualmachine* v,char * property);
    bool is64BitArchitecture(ivirtualmachine* v);
    int getMajorOSVersion(ivirtualmachine* v);
    
#ifdef __cplusplus
}
#endif



#endif
