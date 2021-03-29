
#include "mxj_common.h"
#include "mxj_utils.h"
#include "mxj_attr.h"

#include "Atom.h"
#include "maxjava.h"
#include "ExceptionUtils.h"

extern t_symbol *J_BOOL;
extern t_symbol *J_BYTE;
extern t_symbol *J_CHAR;
extern t_symbol *J_SHORT;
extern t_symbol *J_INT;
extern t_symbol *J_LONG;
extern t_symbol *J_FLOAT;
extern t_symbol *J_DOUBLE;
extern t_symbol *J_STRING;
extern t_symbol *J_BOOL_ARRAY;
extern t_symbol *J_BYTE_ARRAY;
extern t_symbol *J_CHAR_ARRAY;
extern t_symbol *J_SHORT_ARRAY;
extern t_symbol *J_INT_ARRAY;
extern t_symbol *J_LONG_ARRAY;
extern t_symbol *J_FLOAT_ARRAY;
extern t_symbol *J_DOUBLE_ARRAY;
extern t_symbol *J_STRING_ARRAY; 
 
extern t_symbol *ATTR_VIRTUAL;	
extern t_symbol *MAX_LONG;
extern t_symbol *MAX_FLOAT;
extern t_symbol *MAX_SYM;

extern t_symbol *SYM_TRUE;
extern t_symbol *SYM_FALSE;

extern jclass g_stringClass;

extern jmethodID g_getAtomType_MID;
extern jmethodID g_getIntValue_MID;
extern jmethodID g_getFloatValue_MID;
extern jmethodID g_getStringValue_MID;

void mxj_attr_cantget(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_atom(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_atom_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_bool(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_byte(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_char(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_short(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_int(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_long(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_float(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_double(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_string(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);

void mxj_attr_get_bool_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_byte_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_char_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_short_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_int_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_long_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_float_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_double_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);
void mxj_attr_get_string_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv);

void mxj_attr_cantset(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_virtual(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_bool(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_byte(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_char(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_short(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_int(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_long(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_float(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_double(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_string(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);

void mxj_attr_set_bool_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_byte_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_char_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_short_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_int_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_long_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_float_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_double_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);
void mxj_attr_set_string_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv);

t_mxj_attr* mxj_attr_new(JNIEnv *env,jclass clazz,t_mxj_attr_desc* ad)
{
	t_mxj_attr *a;
	static t_symbol *NADA = NULL;
	if (!NADA)
		NADA = gensym("undef");
	
	a = (t_mxj_attr *)	sysmem_newptr(sizeof(t_mxj_attr));
	a->fid            = 0;
	a->g_mid          = 0; // mid for getter
	a->s_mid          = 0; // mid for getter
	a->name        	  = ad->name ? gensym(ad->name) : NADA;
	a->m_f_type       = ad->m_f_type ? gensym(ad->m_f_type) : NADA; //max field type
	a->j_f_type       = ad->j_f_type ? gensym(ad->j_f_type) : NADA; // java field type
	a->setter_jptypes = ad->setter_jptypes  ? gensym(ad->setter_jptypes) : NADA;
	a->getter_jptypes = ad->getter_jptypes? gensym(ad->getter_jptypes) : NADA;
	a->settable       = ad->settable;
	a->gettable	      = ad->gettable; 
	a->isvirtual      = ad->isvirtual;	
	
	if (a->gettable) {
		t_symbol* comp;
		
		if (ad->getter) {	// getter function provided
			char* tmp;

			a->g_mid = MXJ_JNI_CALL(env,GetMethodID)(env, clazz,ad->getter,ad->getter_sig);
			strcpy(a->name_getter,ad->getter);
			
			strcpy(a->name_getter,ad->getter);					
			tmp = (ad->getter_sig) + 2;	// jump past () part of sig. eg ()[I
			if (!strcmp(tmp,"Ljava/lang/String;"))
				comp = J_STRING;
			else if (!strcmp(tmp,"[Ljava/lang/String;"))
				comp = J_STRING_ARRAY;
			else if (!strcmp(tmp,"Lcom/cycling74/max/Atom;")) {
				a->getter = (method)mxj_attr_get_atom;				
				goto getterisset;
			}	
			else if (!strcmp(tmp,"[Lcom/cycling74/max/Atom;")) {	// Atom array
				a->getter = (method)mxj_attr_get_atom_array;				
				goto getterisset;
			}
			else
				comp = gensym(tmp);
		}
		else {
			char* java_sig;
			
			if (!strcmp(ad->j_f_type,"s"))	// we need to add field_sig sometime AttributeInfo.java
				java_sig = "Ljava/lang/String;";
			else if (!strcmp(ad->j_f_type,"[s"))	// we need to add field_sig sometime AttributeInfo.java
				java_sig = "[Ljava/lang/String;";	
			else
				java_sig = ad->j_f_type;
			
			a->fid = MXJ_JNI_CALL(env,GetFieldID)(env, clazz, ad->name,java_sig); 
			strcpy(a->name_getter,"auto");
			comp = a->j_f_type;
		}
						
		// now bind an auto getter function
		if (comp == J_INT)
			a->getter = (method)mxj_attr_get_int;
		else if (comp == J_FLOAT)
			a->getter = (method)mxj_attr_get_float;	
		else if (comp == J_STRING)
			a->getter = (method)mxj_attr_get_string;
		else if (comp == J_DOUBLE)
			a->getter = (method)mxj_attr_get_double;
		else if (comp == J_LONG)
			a->getter = (method)mxj_attr_get_long;
		else if (comp == J_BOOL)
			a->getter = (method)mxj_attr_get_bool;
		else if (comp == J_CHAR)
			a->getter = (method)mxj_attr_get_char;
		else if (comp == J_BYTE)
			a->getter = (method)mxj_attr_get_byte;
		else if (comp == J_SHORT)
			a->getter = (method)mxj_attr_get_short;	
		//arrays	
		else if (comp == J_INT_ARRAY)
			a->getter = (method)mxj_attr_get_int_array;
		else if (comp == J_FLOAT_ARRAY)
			a->getter = (method)mxj_attr_get_float_array;	
		else if (comp == J_STRING_ARRAY)
			a->getter = (method)mxj_attr_get_string_array;
		else if (comp == J_DOUBLE_ARRAY)
			a->getter = (method)mxj_attr_get_double_array;
		else if (comp == J_LONG_ARRAY)
			a->getter = (method)mxj_attr_get_long_array;
		else if (comp == J_BOOL_ARRAY)
			a->getter = (method)mxj_attr_get_bool_array;
		else if (comp == J_CHAR_ARRAY)
			a->getter = (method)mxj_attr_get_char_array;
		else if (comp == J_BYTE_ARRAY)
			a->getter = (method)mxj_attr_get_byte_array;
		else if (comp == J_SHORT_ARRAY)
			a->getter = (method)mxj_attr_get_short_array;			
		
				
	}
	else {	// end isgettable
		a->getter = (method)mxj_attr_cantget;
	}
	
	getterisset:
	
	if (a->settable) {
		if (ad->setter) {
			a->setter = (method)mxj_attr_set_virtual;//user overrides default or has defined attr which is not member variable
			a->s_mid  = MXJ_JNI_CALL(env,GetMethodID)(env,clazz,ad->setter,ad->setter_sig);
			strcpy(a->name_setter,ad->setter);
		}
		else {	// bind to default setter
			if (!a->isvirtual) {
				char* java_sig;
				if (!strcmp(ad->j_f_type,"s")) 
					java_sig = "Ljava/lang/String;";
				else if (!strcmp(ad->j_f_type,"[s")) 
					java_sig = "[Ljava/lang/String;";	
				else
					java_sig = ad->j_f_type;
				a->fid = MXJ_JNI_CALL(env,GetFieldID)(env, clazz, ad->name,java_sig); 
				strcpy(a->name_setter,"auto");
				
				//now bind an auto setter function
				if (a->j_f_type == J_INT)
					a->setter = (method)mxj_attr_set_int;
				else if (a->j_f_type == J_FLOAT)
					a->setter = (method)mxj_attr_set_float;	
				else if (a->j_f_type == J_STRING)
					a->setter = (method)mxj_attr_set_string;
				else if (a->j_f_type == J_DOUBLE)
					a->setter = (method)mxj_attr_set_double;
				else if (a->j_f_type == J_LONG)
					a->setter = (method)mxj_attr_set_long;
				else if (a->j_f_type == J_BOOL)
					a->setter = (method)mxj_attr_set_bool;
				else if (a->j_f_type == J_CHAR)
					a->setter = (method)mxj_attr_set_char;
				else if (a->j_f_type == J_BYTE)
					a->setter = (method)mxj_attr_set_byte;
				else if (a->j_f_type == J_SHORT)
					a->setter = (method)mxj_attr_set_short;					
				if (a->j_f_type == J_INT_ARRAY)
					a->setter = (method)mxj_attr_set_int_array;
				else if (a->j_f_type == J_FLOAT_ARRAY)
					a->setter = (method)mxj_attr_set_float_array;	
				else if (a->j_f_type == J_STRING_ARRAY)
					a->setter = (method)mxj_attr_set_string_array;
				else if (a->j_f_type == J_DOUBLE_ARRAY)
					a->setter = (method)mxj_attr_set_double_array;
				else if (a->j_f_type == J_LONG_ARRAY)
					a->setter = (method)mxj_attr_set_long_array;
				else if (a->j_f_type == J_BOOL_ARRAY)
					a->setter = (method)mxj_attr_set_bool_array;
				else if (a->j_f_type == J_CHAR_ARRAY)
					a->setter = (method)mxj_attr_set_char_array;
				else if (a->j_f_type == J_BYTE_ARRAY)
					a->setter = (method)mxj_attr_set_byte_array;
				else if (a->j_f_type == J_SHORT_ARRAY)
					a->setter = (method)mxj_attr_set_short_array;
			}
			else {
				error("how is a virtual attr settable without a setter?");
				return NULL;	// err
			}	
		}	
	}	// end is settable
	else {
		a->setter = (method)mxj_attr_cantset;
	}
			
	return a;
}

void mxj_attr_cantget(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{	
	post("%s attribute %s is not gettable",x->p_classname,a->name->s_name);
	(*argc) = 0;
	(*argv) = 0;
}

void mxj_attr_cantset(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	post("%s attribute %s is not settable",x->p_classname,a->name->s_name);
}

void mxj_attr_get_atom(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	int javaAtomType;
	jobject javaAtom;

	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));
			
	if (a->fid)
		javaAtom =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance,a->fid);
	else
		javaAtom =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance,a->g_mid);	
	
	
	javaAtomType = MXJ_JNI_CALL(env,CallIntMethod)(env, javaAtom, g_getAtomType_MID);
		switch (javaAtomType) {
			case ATOM_TYPENUM_LONG: {
				long value = MXJ_JNI_CALL(env,CallIntMethod)(env, javaAtom, g_getIntValue_MID);
				A_SETLONG(*argv, value);
				break;
			}
			
			case ATOM_TYPENUM_FLOAT: {
				A_SETFLOAT(*argv, MXJ_JNI_CALL(env,CallFloatMethod)(env, javaAtom, g_getFloatValue_MID));
			    break;
		    }
			    
		    case ATOM_TYPENUM_STRING: {
		    	jstring str = MXJ_JNI_CALL(env,CallObjectMethod)(env, javaAtom, g_getStringValue_MID);
		    	const char *chars = MXJ_JNI_CALL(env,GetStringUTFChars)(env, str, NULL);
		    	t_symbol *sym = gensym((char *)chars);
		    	A_SETSYM(*argv, sym);
		    	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env, str, chars);
		    	MXJ_JNI_CALL(env,DeleteLocalRef)(env,str);
		    	break;
	    	}
	    	

    	}

	*argc = 1;

}

//in java this returns an atom array
void mxj_attr_get_atom_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jobjectArray array;
	
	if (a->fid)
		array = (jobjectArray) MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance,a->fid);
	else
		array = (jobjectArray) MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance,a->g_mid);	
	
	(*argv) = newArgv(env,array,argc);	// this call sets argc as a side effect
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,array);
}

void mxj_attr_set_virtual(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	call_method_with_coercion(x,a->s_mid, a->setter_jptypes,argc,argv);
}


//auto getters
void mxj_attr_get_bool(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jboolean b;
			
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));
	
	if (a->fid) {		
 		b =  MXJ_JNI_CALL(env,GetBooleanField)(env, x->javaInstance, a->fid);
	}
	else
		b = MXJ_JNI_CALL(env,CallBooleanMethod)(env, x->javaInstance, a->g_mid);
		
	if (b == JNI_TRUE)
		A_SETLONG(((t_atom*)*argv),1);
	else
		A_SETLONG(((t_atom*)*argv),0);
	*argc = 1;		
	
}

void mxj_attr_get_bool_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jarray ja;
	int array_len;	
	jboolean* elements;
	jboolean is_copy; 
	int i;
	
	if (a->fid)
 		ja =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
 	else
 		ja =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
 
 	checkException(env);
	
 	if (ja == NULL) {	
 		*argc = 0;
 		*argv = 0;
 		return;
 	}
 	
 	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
	
 	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(array_len * sizeof(t_atom));
	
	elements = MXJ_JNI_CALL(env, GetBooleanArrayElements)(env,(jbooleanArray)ja,&is_copy);
	
	checkException(env);

	for(i = 0; i < array_len;i++)
	{	
		if (elements[i] == JNI_TRUE)
			A_SETLONG(((t_atom *)*argv + i),1);
		else
			A_SETLONG(((t_atom *)*argv + i),0);	
	}
	
	if (is_copy)
	{
		MXJ_JNI_CALL(env, ReleaseBooleanArrayElements)(env,(jbooleanArray)ja,elements,0);
		checkException(env);
	}
	*argc = array_len;		
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
}

void mxj_attr_get_byte(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	long b;
		
	
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));	
	
	if (a->fid) 
 		b =  (long)MXJ_JNI_CALL(env,GetByteField)(env, x->javaInstance, a->fid);
	else
		b =  (long)MXJ_JNI_CALL(env,CallByteMethod)(env, x->javaInstance, a->g_mid);

	A_SETLONG(((t_atom*)*argv),b);
	*argc = 1;		
	
}

void mxj_attr_get_byte_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jarray ja;
	int array_len;	
	jbyte* elements;
	jboolean is_copy; 
	int i;
	
	if (a->fid) 
 		ja =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
 	else
 		ja =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
 		
 	checkException(env);
	
  	if (ja == NULL) {	
 		*argc = 0;
 		*argv = 0;
 		return;
 	}
 
 	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
 	
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(array_len * sizeof(t_atom));
	
	elements = MXJ_JNI_CALL(env, GetByteArrayElements)(env,(jbyteArray)ja,&is_copy);
	checkException(env);

	for(i = 0; i < array_len;i++)	
		A_SETLONG(((t_atom *)*argv + i),(long)elements[i]);
	
	if (is_copy) {
		MXJ_JNI_CALL(env, ReleaseByteArrayElements)(env,(jbyteArray)ja,elements,0);
		checkException(env);
	}
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);

	*argc = array_len;
}

void mxj_attr_get_char(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	long c;
			
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));	
 
 	if (a->fid) 
 		c = (long) MXJ_JNI_CALL(env,GetCharField)(env, x->javaInstance, a->fid);
	else
		c = (long) MXJ_JNI_CALL(env,CallCharMethod)(env, x->javaInstance, a->g_mid);
		
	A_SETLONG(((t_atom*)*argv),c);
	*argc = 1;		
}

void mxj_attr_get_char_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jarray ja;
	int array_len;	
	jchar* elements;
	jboolean is_copy; 
	int i;
	
	if (a->fid) 
 		ja =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
 	else
		ja = MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
 	 	
 	checkException(env);

  	if (ja == NULL) {
 		*argc = 0;
 		*argv = 0;
 		return;
 	}
 
 	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
 	
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(array_len * sizeof(t_atom));
	
	elements = MXJ_JNI_CALL(env, GetCharArrayElements)(env,(jcharArray)ja,&is_copy);
	checkException(env);

	for(i = 0; i < array_len;i++)	
		A_SETLONG(((t_atom *)*argv + i),(long)elements[i]);
	
	if (is_copy) {
		MXJ_JNI_CALL(env, ReleaseCharArrayElements)(env,(jcharArray)ja,elements,0);
		checkException(env);
	}
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
	*argc = array_len;	
}


void mxj_attr_get_short(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	long s;
			
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));	
	if (a->fid) 	
 		s = (long) MXJ_JNI_CALL(env,GetShortField)(env, x->javaInstance, a->fid);
	else
		s = (long) MXJ_JNI_CALL(env,CallShortMethod)(env, x->javaInstance, a->g_mid);
	
	A_SETLONG(((t_atom*)*argv),s);
	*argc = 1;
}

void mxj_attr_get_short_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{

	jarray ja;
	int array_len;	
	jshort* elements;
	jboolean is_copy; 
	int i;
	
	if (a->fid) 
 		ja =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
 	else
 		ja =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
 	
 	checkException(env);
 	
 	if (ja == NULL) {	
 		*argc = 0;
 		*argv = 0;
 		return;
 	}
 	
 	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
 	
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(array_len * sizeof(t_atom));
	
	elements = MXJ_JNI_CALL(env, GetShortArrayElements)(env,(jshortArray)ja,&is_copy);
	checkException(env);

	for(i = 0; i < array_len;i++)	
		A_SETLONG(((t_atom *)*argv + i),(long)elements[i]);
	
	if (is_copy) {
		MXJ_JNI_CALL(env, ReleaseShortArrayElements)(env,(jshortArray)ja,elements,0);
		checkException(env);
	}
	
	*argc = array_len;
}


void mxj_attr_get_int(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	long i;
			
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));
	
	if (a->fid)				
 		i = (long) MXJ_JNI_CALL(env,GetIntField)(env, x->javaInstance, a->fid);
	else
		i = (long) MXJ_JNI_CALL(env,CallIntMethod)(env, x->javaInstance, a->g_mid);
		
	A_SETLONG(((t_atom*)*argv),i);
	*argc = 1;			
}

void mxj_attr_get_int_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jarray ja;
	int array_len;	
	jint* elements;
	jboolean is_copy; 
	int i;
	
	if (a->fid)
 		ja =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
 	else
 		ja =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
 	
 	checkException(env);
 	
 	if (ja == NULL) 	{	
 		*argc = 0;
 		*argv = 0;
 		return;
 	}
	
 	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
 	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(array_len * sizeof(t_atom));
	
	elements = MXJ_JNI_CALL(env, GetIntArrayElements)(env,(jintArray)ja,&is_copy);
	checkException(env);

	for(i = 0; i < array_len;i++)	
		A_SETLONG(((t_atom *)*argv + i),(long)elements[i]);
	
	if (is_copy) {
		MXJ_JNI_CALL(env, ReleaseIntArrayElements)(env,(jintArray)ja,elements,0);
		checkException(env);
	}
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
	*argc = array_len;
}


void mxj_attr_get_long(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	long l;
			
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));	
	
	if (a->fid)
 		l = (long) MXJ_JNI_CALL(env,GetLongField)(env, x->javaInstance, a->fid);
	else
		l = (long) MXJ_JNI_CALL(env,CallLongMethod)(env, x->javaInstance, a->g_mid);
	
	A_SETLONG(((t_atom*)*argv),l);
	*argc = 1;
}

void mxj_attr_get_long_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jarray ja;
	int array_len;	
	jlong* elements;
	jboolean is_copy; 
	int i;
	
 	if (a->fid)
 		ja =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
 	else
 		ja = MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
 	
 	checkException(env);
 	if (ja == NULL) {	
 		*argc = 0;
 		*argv = 0;
 		return;
 	}
 	
 	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
 	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(array_len * sizeof(t_atom));
	
	elements = MXJ_JNI_CALL(env, GetLongArrayElements)(env,(jlongArray)ja,&is_copy);
	checkException(env);

	for(i = 0; i < array_len;i++)	
		A_SETLONG(((t_atom *)*argv + i),(long)elements[i]);
	
	if (is_copy) {
		MXJ_JNI_CALL(env, ReleaseLongArrayElements)(env,(jlongArray)ja,elements,0);
		checkException(env);
	}
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
	*argc = array_len;
}

void mxj_attr_get_float(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	double f;
			
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));
		
	if (a->fid)		
 		f = (double) MXJ_JNI_CALL(env,GetFloatField)(env, x->javaInstance, a->fid);
	else
		f = (double) MXJ_JNI_CALL(env,CallFloatMethod)(env, x->javaInstance, a->g_mid);
	
	A_SETFLOAT(((t_atom*)*argv),(t_atom_float)f);
	*argc = 1;		
}

void mxj_attr_get_float_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jarray ja;
	int array_len;	
	jfloat* elements;
	jboolean is_copy; 
	int i;
	
	if (a->fid)
 		ja =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
 	else
 		ja =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
 	
 	checkException(env);
 	if (ja == NULL) {	
 		*argc = 0;
 		*argv = 0;
 		return;
 	}
 	
 	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
 	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(array_len * sizeof(t_atom));
	
	elements = MXJ_JNI_CALL(env, GetFloatArrayElements)(env,(jfloatArray)ja,&is_copy);
	checkException(env);

	for(i = 0; i < array_len;i++)	
		A_SETFLOAT(((t_atom *)*argv + i),(float)elements[i]);
	
	if (is_copy) {
		MXJ_JNI_CALL(env, ReleaseFloatArrayElements)(env,(jfloatArray)ja,elements,0);
		checkException(env);
	}
	
	*argc = array_len;
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
}

void mxj_attr_get_double(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	double d;
			
	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));	
	
	if (a->fid)	
 		d = (double) MXJ_JNI_CALL(env,GetDoubleField)(env, x->javaInstance, a->fid);
	else
		d = (double) MXJ_JNI_CALL(env,CallDoubleMethod)(env, x->javaInstance, a->g_mid);
	
	A_SETFLOAT(((t_atom*)*argv),(t_atom_float)d);
	*argc = 1;			
}

void mxj_attr_get_double_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jarray ja;
	int array_len;	
	jdouble* elements;
	jboolean is_copy; 
	int i;
	
	if (a->fid)
 		ja =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
 	else
 		ja =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
 	
 	checkException(env);
 	if (ja == NULL) {	
 		*argc = 0;
 		*argv = 0;
 		return;
 	}
 	
 	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
 	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(array_len * sizeof(t_atom));
	
	elements = MXJ_JNI_CALL(env, GetDoubleArrayElements)(env,(jdoubleArray)ja,&is_copy);
	checkException(env);

	for(i = 0; i < array_len;i++)	
		A_SETFLOAT(((t_atom *)*argv + i),(float)elements[i]);
	
	if (is_copy) {
		MXJ_JNI_CALL(env, ReleaseDoubleArrayElements)(env,(jdoubleArray)ja,elements,0);
		checkException(env);
	}
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
	*argc = array_len;
}


void mxj_attr_get_string(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jstring js;
	char *s;
			
	if (a->fid)
 		js =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
	else
		js =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
		
	if (js == NULL) {
 		*argc = 0;
 		*argv = 0;	
 		return;
 	}
 	
 	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(sizeof(t_atom));	
	s = (char*)MXJ_JNI_CALL(env,GetStringUTFChars)(env,js,NULL);
	A_SETSYM(((t_atom*)*argv),gensym(s));
	MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,js,s);
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,js);
	*argc = 1;			
}

void mxj_attr_get_string_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short *argc, t_atom **argv)
{
	jarray ja;
	int array_len;	
	jstring js;
	const char* ps;
	int i;
	
	if (a->fid)
 		ja =  MXJ_JNI_CALL(env,GetObjectField)(env, x->javaInstance, a->fid);
 	else
 		ja =  MXJ_JNI_CALL(env,CallObjectMethod)(env, x->javaInstance, a->g_mid);
 		
 	checkException(env);
 	
 	if (ja == NULL) 	{
 		*argc = 0;
 		*argv = 0;	
 		return;
 	}
	
 	array_len = MXJ_JNI_CALL(env,GetArrayLength)(env,ja);
 	checkException(env);
 	if (!*argv)
		*argv = (t_atom *)mxj_getbytes(array_len * sizeof(t_atom));
	
	for(i = 0; i < array_len;i++) {	
		js = (jstring)MXJ_JNI_CALL(env, GetObjectArrayElement)(env,(jobjectArray)ja,i);
		checkException(env);
		ps = MXJ_JNI_CALL(env,GetStringUTFChars)(env,js,NULL);
		checkException(env);		
		A_SETSYM(((t_atom *)*argv + i),gensym((char *)ps));
		MXJ_JNI_CALL(env,ReleaseStringUTFChars)(env,js,ps);
		MXJ_JNI_CALL(env,DeleteLocalRef)(env,js);		
	}
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
	*argc = array_len;
}

//auto setters
void mxj_attr_set_bool(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
 	MXJ_JNI_CALL(env,SetBooleanField)(env, x->javaInstance, a->fid,atomargs_getboolean(0,argc,argv));
	checkException(env);
}

void mxj_attr_set_bool_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jbooleanArray ar;
	int q;
	void *vp;
	
	ar = MXJ_JNI_CALL(env,NewBooleanArray)(env,argc);
	checkException(env);
	
	vp = (jboolean *)mxj_getbytes(argc * sizeof(jboolean));				
	
	for(q = 0;q < argc;q++)
		((jboolean *)vp)[q] = atomargs_getboolean(q,argc,argv);

	MXJ_JNI_CALL(env,SetBooleanArrayRegion)(env,ar,0,argc,(jboolean *)vp);
 	checkException(env);
 	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, a->fid,(jobject)ar);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ar);
	mxj_freebytes(vp, argc * sizeof(jboolean));
}

void mxj_attr_set_byte(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
 	MXJ_JNI_CALL(env,SetByteField)(env, x->javaInstance, a->fid,(jbyte)atomargs_getlong(0,argc,argv));
	checkException(env);
}

void mxj_attr_set_byte_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jbyteArray ar;
	int q;
	void *vp;
	
	ar = MXJ_JNI_CALL(env,NewByteArray)(env,argc);
	checkException(env);
	
	vp = (jbyte *)mxj_getbytes(argc * sizeof(jbyte));				
	
	for(q = 0;q < argc;q++)
		((jbyte *)vp)[q] = (jbyte)atomargs_getlong(q,argc,argv);

	MXJ_JNI_CALL(env,SetByteArrayRegion)(env,ar,0,argc,(jbyte *)vp);
 	checkException(env);
 	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, a->fid,(jobject)ar);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ar);
	mxj_freebytes(vp, argc * sizeof(jbyte));
}

void mxj_attr_set_char(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
 	MXJ_JNI_CALL(env,SetCharField)(env, x->javaInstance, a->fid,(jchar)atomargs_getlong(0,argc,argv));
	checkException(env);
}

void mxj_attr_set_char_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jcharArray ar;
	int q;
	void *vp;
	
	ar = MXJ_JNI_CALL(env,NewCharArray)(env,argc);
	checkException(env);
	
	vp = (jchar *)mxj_getbytes(argc * sizeof(jchar));				
	
	for(q = 0;q < argc;q++)
		((jchar *)vp)[q] = (jchar)atomargs_getchar(q,argc,argv);

	MXJ_JNI_CALL(env,SetCharArrayRegion)(env,ar,0,argc,(jchar *)vp);
 	checkException(env);
 	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, a->fid,(jobject)ar);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ar);
	mxj_freebytes(vp, argc * sizeof(jchar));
}

void mxj_attr_set_short(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
 	MXJ_JNI_CALL(env,SetShortField)(env, x->javaInstance, a->fid,(jshort)atomargs_getlong(0,argc,argv));
	checkException(env);
}

void mxj_attr_set_short_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jshortArray ar;
	int q;
	void *vp;
	
	ar = MXJ_JNI_CALL(env,NewShortArray)(env,argc);
	checkException(env);
	
	vp = (jshort *)mxj_getbytes(argc * sizeof(jshort));				
	
	for(q = 0;q < argc;q++)
		((jshort *)vp)[q] = (jshort)atomargs_getlong(q,argc,argv);

	MXJ_JNI_CALL(env,SetShortArrayRegion)(env,ar,0,argc,(jshort *)vp);
 	checkException(env);
 	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, a->fid,(jobject)ar);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ar);
	mxj_freebytes(vp, argc * sizeof(jshort));
}

void mxj_attr_set_int(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
 	MXJ_JNI_CALL(env,SetIntField)(env, x->javaInstance, a->fid,(jint)atomargs_getlong(0,argc,argv));
	checkException(env);
}

void mxj_attr_set_int_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jintArray ar;
	int q;
	void *vp;
	
	ar = MXJ_JNI_CALL(env,NewIntArray)(env,argc);
	checkException(env);
	
	vp = (jint *)mxj_getbytes(argc * sizeof(jint));				
	
	for(q = 0;q < argc;q++)
		((jint *)vp)[q] = (jint)atomargs_getlong(q,argc,argv);

	MXJ_JNI_CALL(env,SetIntArrayRegion)(env,ar,0,argc,(jint *)vp);
 	checkException(env);
 	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, a->fid,(jobject)ar);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ar);
	mxj_freebytes(vp, argc * sizeof(jint));
}

void mxj_attr_set_long(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
 	MXJ_JNI_CALL(env,SetLongField)(env, x->javaInstance, a->fid,(jlong)atomargs_getlong(0,argc,argv));
	checkException(env);
}

void mxj_attr_set_long_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jlongArray ar;
	int q;
	void *vp;
	
	ar = MXJ_JNI_CALL(env,NewLongArray)(env,argc);
	checkException(env);
	
	vp = (jlong *)mxj_getbytes(argc * sizeof(jlong));				
	
	for(q = 0;q < argc;q++)
		((jlong *)vp)[q] = (jlong)atomargs_getlong(q,argc,argv);

	MXJ_JNI_CALL(env,SetLongArrayRegion)(env,ar,0,argc,(jlong *)vp);
 	checkException(env);
 	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, a->fid,(jobject)ar);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ar);
	mxj_freebytes(vp, argc * sizeof(jlong));
}

void mxj_attr_set_float(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
 	MXJ_JNI_CALL(env,SetFloatField)(env, x->javaInstance, a->fid,(jfloat)atomargs_getfloat(0,argc,argv));
	checkException(env);
}

void mxj_attr_set_float_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jfloatArray ar;
	int q;
	void *vp;
	
	ar = MXJ_JNI_CALL(env,NewFloatArray)(env,argc);
	checkException(env);
	
	vp = (jfloat *)mxj_getbytes(argc * sizeof(jfloat));				
	
	for(q = 0;q < argc;q++)
		((jfloat *)vp)[q] = (jfloat)atomargs_getfloat(q,argc,argv);

	MXJ_JNI_CALL(env,SetFloatArrayRegion)(env,ar,0,argc,(jfloat *)vp);
 	checkException(env);
 	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, a->fid,(jobject)ar);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ar);
	mxj_freebytes(vp, argc * sizeof(jfloat));
}

void mxj_attr_set_double(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
 	MXJ_JNI_CALL(env,SetDoubleField)(env, x->javaInstance, a->fid,(jdouble)atomargs_getfloat(0,argc,argv));
	checkException(env);
}

void mxj_attr_set_double_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jdoubleArray ar;
	int q;
	void *vp;
	
	ar = MXJ_JNI_CALL(env,NewDoubleArray)(env,argc);
	checkException(env);
	
	vp = (jdouble *)mxj_getbytes(argc * sizeof(jdouble));				
	
	for(q = 0;q < argc;q++)
		((jdouble *)vp)[q] = (jdouble)atomargs_getfloat(q,argc,argv);

	MXJ_JNI_CALL(env,SetDoubleArrayRegion)(env,ar,0,argc,(jdouble *)vp);
 	checkException(env);
 	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, a->fid,(jobject)ar);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ar);
	mxj_freebytes(vp, argc * sizeof(jdouble));
}

void mxj_attr_set_string(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jstring js;
	
	js = MXJ_JNI_CALL(env,NewStringUTF)(env,(const char*)((atomargs_getsym(0,argc,argv))->s_name));
 	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance, a->fid,js);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,js);
}

void mxj_attr_set_string_array(JNIEnv *env, t_maxjava *x, t_mxj_attr *a, short argc, t_atom *argv)
{
	jarray ja;
	jstring js;
	int q;
	
	ja  = (jarray)MXJ_JNI_CALL(env,NewObjectArray)(env,argc,g_stringClass,NULL);
	checkException(env);
									
	for(q = 0;q < argc;q++) {
		js = (MXJ_JNI_CALL(env,NewStringUTF)(env,(atomargs_getsym(q,argc,argv))->s_name));
		MXJ_JNI_CALL(env,SetObjectArrayElement)(env,ja,q,js);  	
		MXJ_JNI_CALL(env,DeleteLocalRef)(env,js);
	}
	
	checkException(env);													
	MXJ_JNI_CALL(env,SetObjectField)(env, x->javaInstance,a->fid,ja);
	checkException(env);
	
	MXJ_JNI_CALL(env,DeleteLocalRef)(env,ja);
}

void mxj_attr_free(t_mxj_attr* x)
{
	sysmem_freeptr(x);
}
