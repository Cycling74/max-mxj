#ifndef _Included_mjglobals_h
#define _Included_mjglobals_h


/*
 * Global data for use by various parts of the system.
 */

// Class com.cycling74.max.Atom
jclass g_atomClass;

jclass g_stringClass;
jclass g_ObjectClass;//java/lang/Object
#ifdef MXJ_MSP
	jclass g_msp_sig_clz;//MSPSignal class ref
	jmethodID g_msp_sig_const;//MSPSignal constructor
	jobject EMPTY_MSPSIG_ARRAY;
	
	jobject real_java_obj_array_ref;
	
	//jfieldID g_msp_sig_buf_fid;//MSPSignal.buf
	//jfieldID g_msp_sig_vec_fid;//vec
	//jfieldID g_msp_sig_sr_fid;//sr
	//jfieldID g_msp_sig_n_fid;//vec_size
	//jfieldID g_msp_sig_cc_fid;//connection count
	//jfieldID g_msp_sig_c_fid;//connected flag
#endif
// Method com.cycling74.max.Atom.newAtom(int)
jmethodID g_newIntAtom_MID;
// Method com.cycling74.max.Atom.newAtom(float)
jmethodID g_newFloatAtom_MID;
// Method com.cycling74.max.Atom.newAtom(String)
jmethodID g_newStringAtom_MID;
// Method com.cycling74.max.Atom.getType
jmethodID g_getAtomType_MID;
// Method com.cycling74.max.IntAtom.getInt
jmethodID g_getIntValue_MID;
// Method com.cycling74.max.FloatAtom.getFloat
jmethodID g_getFloatValue_MID;
// Method com.cycling74.max.StringAtom.getString
jmethodID g_getStringValue_MID;



#endif
