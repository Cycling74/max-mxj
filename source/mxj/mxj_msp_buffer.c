
#ifdef MAC_VERSION
#include "jni.h"        // Java Native Interface definitions
#include "jni_md.h"
#else
#include "jni.h"
#include "jni_md.h"
#endif

#include "classes.h"
#include "callbacks.h"
#include "ExceptionUtils.h"
#include "mxj_msp_buffer.h"
#include "ext_buffer.h"
#include "z_sampletype.h"

#define MXJ_MSPBUF_CLASSNAME  "com/cycling74/msp/MSPBuffer"

extern JavaVM	*g_jvm;

static jclass	s_mspbuf_clazz;


/* generic Max class to bind buffer references to */

typedef struct _mxj_buf_ref
{
	t_object		a_obj;
} t_mxj_buf_ref;

t_class *s_mxj_buf_ref_class = NULL;
static t_object *s_mxj_buf_ref = NULL;

void mxj_buf_ref_initclass(void);

t_mxj_buf_ref* mxj_buf_ref_new()
{
	t_mxj_buf_ref *x;
	
	if (!s_mxj_buf_ref_class)
		mxj_buf_ref_initclass();
	
	x = (t_mxj_buf_ref *) object_alloc(s_mxj_buf_ref_class);
	
	return x;
}

void mxj_buf_ref_free(t_mxj_buf_ref *x)
{
}

void mxj_buf_ref_initclass(void)
{
	t_class *c;
	
	if (!s_mxj_buf_ref_class) {
		c = class_new("mxj_buf_ref", (method)mxj_buf_ref_new, (method)mxj_buf_ref_free, sizeof(t_mxj_buf_ref), (method)NULL, A_CANT, 0L);
		s_mxj_buf_ref_class = c;
		class_register(CLASS_NOBOX, c);
	}
}

/* end generic Max class to bind buffer references to */


//internal functions
void mxj_mspbuf_register_natives(JNIEnv*,jclass );

//JNI bound functions

//jlong JNICALL mxj_mspbuf_get_ptr(JNIEnv *env,jclass clazz, jstring name);
jint JNICALL mxj_mspbuf_numchans(JNIEnv * env, jclass cls, jstring name);
//get size in milliseconds
jdouble JNICALL mxj_mspbuf_get_length(JNIEnv * env, jclass cls, jstring name);
//set length in milliseconds
void JNICALL mxj_mspbuf_set_length(JNIEnv* env, jclass cls, jstring name, jint numchannels, jdouble length);
//get framecount in samples
jlong JNICALL mxj_mspbuf_get_frames(JNIEnv* env, jclass cls, jstring name);
//set framecount in samples
void JNICALL mxj_mspbuf_set_frames(JNIEnv* env, jclass cls, jstring name,  jint numchannels, jlong frames);
//get size in samples
jlong JNICALL mxj_mspbuf_get_size(JNIEnv* env, jclass cls, jstring name);
//set size in samples
void JNICALL mxj_mspbuf_set_size(JNIEnv* env, jclass cls, jstring name, jint numchannels, jlong frames);
t_buffer_ref *mxj_mspbuf_is_valid(JNIEnv *env, jclass cls, jstring name);
void mxj_mspbuf_read_channel(float *read, float *write, int channels, long size);
void mxj_mspbuf_write_channel(float *read, float *write, int channels, long size);
jfloatArray JNICALL mxj_mspbuf_peek_region(JNIEnv *env, jclass cls, jstring name, jint channel, jlong start, jlong length);
jfloat JNICALL mxj_mspbuf_peek(JNIEnv *env, jclass cls, jstring name, jint channel, jlong n);
jfloatArray JNICALL mxj_mspbuf_peek_channel(JNIEnv *env, jclass cls, jstring name, jint channel);
jfloatArray JNICALL mxj_mspbuf_peek_all(JNIEnv *env, jclass cls, jstring name);
void JNICALL mxj_mspbuf_poke(JNIEnv *env, jclass cls, jstring name, jint chan,jlong index,jfloat value);
void JNICALL mxj_mspbuf_poke_region(JNIEnv *env, jclass cls, jstring name, jint channel, jlong start, jfloatArray data_array);
void JNICALL mxj_mspbuf_poke_channel(JNIEnv *env, jclass cls, jstring name, jint channel, jfloatArray data_array);
void JNICALL mxj_mspbuf_poke_all(JNIEnv *env, jclass cls, jstring name, jfloatArray data_array);
void mxj_mspbuf_dirtybuffer(t_buffer_ref *b);
extern t_symbol *newSym(JNIEnv *env, jstring str);
char* mxj_getbytes(t_atom_long size);
void mxj_freebytes(void *vp, t_atom_long size);



t_buffer_ref *mxj_mspbuf_is_valid(JNIEnv *env, jclass cls, jstring name)
{
	t_symbol *bufname;
    C74_ASSERT(s_mxj_buf_ref);

	if (name == NULL)
		return NULL;
	
	bufname = newSym(env, name);
	
	return buffer_ref_new(s_mxj_buf_ref, bufname);
}


jint JNICALL mxj_mspbuf_numchans(JNIEnv * env, jclass cls, jstring name)
{
	jint numchans = 0;
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			numchans = (jint)buffer_getchannelcount(bo);
		}

		object_free(b);
	}
	
	return numchans;
}

jdouble JNICALL mxj_mspbuf_get_length(JNIEnv * env, jclass cls, jstring name)
{
	jdouble length = -1.;
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			length = (jdouble)((1./ (buffer_getsamplerate(bo) * 0.001)) * buffer_getframecount(bo));
		}
		
		object_free(b);
	}
	
	if (length == -1.)
		error("(mxj) invalid buf");
	
	return length;
}

void JNICALL mxj_mspbuf_set_length(JNIEnv* env, jclass cls, jstring name, jint numchannels, jdouble length)
{
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			object_method_direct(void, (void *, double, long), bo, gensym("setsize"), length, numchannels);
		} else {
			error("(mxj) invalid buf");
		}
		
		object_free(b);
	}
}

jlong JNICALL mxj_mspbuf_get_frames(JNIEnv* env, jclass cls, jstring name)
{
	jsize frames = -1;
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo)
			frames = (jsize)buffer_getframecount(bo);
		object_free(b);
	}
	
	if (frames == -1)
		error("(mxj) invalid buf");

	return frames;
}

void JNICALL mxj_mspbuf_set_frames(JNIEnv* env, jclass cls, jstring name,  jint numchannels, jlong frames)
{
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			object_method_direct(void, (void *, long, long), bo, gensym("sizeinsamps"), (long)frames, numchannels);
		} else {
			error("(mxj) invalid buf");
		}
		
		object_free(b);
	}
}


jlong JNICALL mxj_mspbuf_get_size(JNIEnv* env, jclass cls, jstring name)
{
	jlong size = -1;
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		t_buffer_info info;
		if (bo && buffer_getinfo(bo, &info) == MAX_ERR_NONE) {
			size = info.b_size;
		}
	}
	
	if (size == -1)
		error("(mxj) invalid buf");
	
	return size;
}

void JNICALL mxj_mspbuf_set_size(JNIEnv* env, jclass cls, jstring name,  jint numchannels, jlong size)
{
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			long frames = (long)size/numchannels;
			object_method_direct(void, (void *, long, long), bo, gensym("sizeinsamps"), frames, numchannels);
		} else {
			error("(mxj) invalid buf");
		}
		object_free(b);
	}
}


void mxj_mspbuf_read_channel(float *read, float *write, int channels, long size)
{
	long i;

	for (i=0;i<size;i++) {
		*write++ = *read;
		read+=channels;
	}
}

void mxj_mspbuf_write_channel(float *read, float *write, int channels, long size)
{
	long i;
	
	for (i=0;i<size;i++) {
		*write = *read++;
		write+=channels;
	}
}

jfloat JNICALL mxj_mspbuf_peek(JNIEnv *env, jclass cls, jstring name, jint channel, jlong n)
{
	jfloat result = 0.;
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			t_float	*smps = buffer_locksamples(bo);
			if (smps) {
				jint channelcount = (jint)buffer_getchannelcount(bo);
				jsize framecount = (jsize)buffer_getframecount(bo);
				
				if (n < 0)
					n = 0;
				else if (n >= framecount)
					n = framecount - 1;
				n *= channelcount;
				channel = (jint)CLAMP(channel,1,channelcount) - 1;
				n += channel;
				result = smps[n];
				buffer_unlocksamples(bo);
			}
		}
		
		object_free(b);
	}
	
	return result;
}


jfloatArray JNICALL mxj_mspbuf_peek_region(JNIEnv *env,jclass cls, jstring name, jint channel, jlong start, jlong length)
{
	jfloatArray jfarr;
	float *copiedarray;
	
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);

	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			t_float	*smps = buffer_locksamples(bo);
			
			if (smps) {
				jint channelcount = (jint)buffer_getchannelcount(bo);
				jsize framecount = (jsize)buffer_getframecount(bo);
				
				if (start < 0)
					start = 0;
				if (start+length >= framecount)
					length = framecount - start;
	
				channel = (jint)CLAMP(channel,1,channelcount) - 1;
				
				start = start*channelcount + channel;
				jfarr = (MXJ_JNI_CALL(env,NewFloatArray)(env, (jsize)length));
				
				if (channelcount > 1) {
					copiedarray = (float *)mxj_getbytes(sizeof(float)*(long)length);
					mxj_mspbuf_read_channel(smps+start, copiedarray, (int)channelcount, (long)length);
					MXJ_JNI_CALL(env,SetFloatArrayRegion)(env,jfarr,0,(jsize)length,copiedarray);
					mxj_freebytes(copiedarray, sizeof(float)*(long)length);
				} else {
					MXJ_JNI_CALL(env,SetFloatArrayRegion)(env,jfarr,0,(jsize)length,smps+start);
				}
				
				buffer_unlocksamples(bo);
				object_free(b);
				
				return jfarr;
			}
		}
		
		object_free(b);
	}
	
	return 0;
}

jfloatArray JNICALL mxj_mspbuf_peek_channel(JNIEnv *env, jclass cls, jstring name, jint channel)
{
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			t_float	*smps = buffer_locksamples(bo);
			
			if (smps) {
				jint channelcount = (jint)buffer_getchannelcount(bo);
				jsize frames = (jsize)buffer_getframecount(bo);
				
				float *copiedarray = (float *)mxj_getbytes(sizeof(float)*frames);
				jfloatArray jfarr = (MXJ_JNI_CALL(env,NewFloatArray)(env, frames));
				long start = 0;
				channel = CLAMP(channel,1,channelcount) - 1;
				start += channel;
				mxj_mspbuf_read_channel(smps+start, copiedarray, channelcount, frames);
				MXJ_JNI_CALL(env,SetFloatArrayRegion)(env,jfarr,0,frames,copiedarray);
				mxj_freebytes(copiedarray, sizeof(float)*frames);
				
				buffer_unlocksamples(bo);
				object_free(b);
	
				return jfarr;
			}
		}
		
		object_free(b);
	}
	
	return 0;
}

jfloatArray JNICALL mxj_mspbuf_peek_all(JNIEnv *env, jclass cls, jstring name)
{
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		t_buffer_info info;
		
		if (bo && buffer_getinfo(bo, &info) == MAX_ERR_NONE) {
			t_float	*smps = buffer_locksamples(bo);
			
			if (smps) {
				long size = info.b_size;
				jfloatArray jfarr = (MXJ_JNI_CALL(env,NewFloatArray)(env, (jsize)size));
				MXJ_JNI_CALL(env,SetFloatArrayRegion)(env,jfarr,0,(jsize)size,smps);
				
				buffer_unlocksamples(bo);
				object_free(b);

				return jfarr;
			}
		}
		
		object_free(b);
	}
	
	return 0;
}

void JNICALL mxj_mspbuf_poke(JNIEnv *env, jclass cls, jstring name, jint chan, jlong index, jfloat value)
{
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			t_float	*smps = buffer_locksamples(bo);
			
			if (smps) {
				jint channelcount = (jint)buffer_getchannelcount(bo);
				jsize frames = (jsize)buffer_getframecount(bo);
				
				index = CLAMP((long)index,0,frames-1);
				index *= channelcount;
				chan = CLAMP(chan,1,channelcount) - 1;
				index += chan;
				smps[index] = value;
				
				buffer_unlocksamples(bo);
				mxj_mspbuf_dirtybuffer(b);
			}
		}
		
		object_free(b);
	}
}

void JNICALL mxj_mspbuf_poke_region(JNIEnv *env, jclass cls, jstring name, jint channel, jlong start, jfloatArray data_array)
{
	float *data, *temp;
	long size;
	jboolean iscopy;

	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			t_float	*smps = buffer_locksamples(bo);
			
			if (smps) {
				jint channelcount = (jint)buffer_getchannelcount(bo);
				jsize frames = (jsize)buffer_getframecount(bo);
				
				if (start < 0)
					start = 0;
	
				if (start > frames-1) {
					object_free(b);
				
					return;	// throw an exception here
				}
				
				size = (MXJ_JNI_CALL(env,GetArrayLength)(env,data_array));

				if (size+start > frames)
					size = frames-(long)start;
				
				data = (float *)(MXJ_JNI_CALL(env,GetPrimitiveArrayCritical)(env, data_array, &iscopy));
				temp = data;
				start *= channelcount;
				channel = CLAMP(channel,1,channelcount) - 1;
				start += channel;
				mxj_mspbuf_write_channel(data, smps+start, channelcount, size);
				mxj_mspbuf_dirtybuffer(b);
				(MXJ_JNI_CALL(env,ReleasePrimitiveArrayCritical)(env, data_array,temp ,0));
				buffer_unlocksamples(bo);
			}
		}
		
		object_free(b);
	}
}

void JNICALL mxj_mspbuf_poke_channel(JNIEnv *env, jclass cls, jstring name, jint channel, jfloatArray data_array)
{
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);

	jboolean iscopy;
	
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		
		if (bo) {
			t_float	*smps = buffer_locksamples(bo);
			
			if (smps) {
				jint channelcount = (jint)buffer_getchannelcount(bo);
				jsize frames = (jsize)buffer_getframecount(bo);
				
				long start = 0;
				long size = (MXJ_JNI_CALL(env,GetArrayLength)(env,data_array));
				float *data = (float *)(MXJ_JNI_CALL(env,GetPrimitiveArrayCritical)(env, data_array, &iscopy));
				float *temp = data;
				
				if (size > frames)
					size = frames;
	
				channel = CLAMP(channel,1,channelcount) - 1;
				start += channel;
				mxj_mspbuf_write_channel(data, smps+start, channelcount, size);
				mxj_mspbuf_dirtybuffer(b);
				(MXJ_JNI_CALL(env,ReleasePrimitiveArrayCritical)(env, data_array,temp ,0));
				buffer_unlocksamples(bo);
			}
		}
		
		object_free(b);
	}
}


void JNICALL mxj_mspbuf_poke_all(JNIEnv *env, jclass cls, jstring name, jfloatArray data_array)
{
	t_buffer_ref *b = mxj_mspbuf_is_valid(env, cls, name);
	jboolean iscopy;

	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		t_buffer_info info;
		
		if (bo && buffer_getinfo(bo, &info) == MAX_ERR_NONE) {
			t_float	*smps = buffer_locksamples(bo);
			
			if (smps) {
				long i;
				float *write = smps;
				long size = (MXJ_JNI_CALL(env,GetArrayLength)(env,data_array));
				float *data = (float *)(MXJ_JNI_CALL(env,GetPrimitiveArrayCritical)(env, data_array, &iscopy));
				float *temp = data;
				if (size > info.b_size)
					size = info.b_size;
				for (i=0;i<size;i++)
					*write++ = *data++;
				mxj_mspbuf_dirtybuffer(b);
				(MXJ_JNI_CALL(env,ReleasePrimitiveArrayCritical)(env, data_array,temp ,0));
				buffer_unlocksamples(bo);
			}
		}
		
		object_free(b);
	}
}

void mxj_mspbuf_dirtybuffer(t_buffer_ref *b)
{
	if (b) {
		t_buffer_obj *bo = buffer_ref_getobject(b);
		buffer_setdirty(bo);
	}
}


//get frames
//get channels
//get sr
//open buf window
//close buf window??

void init_mxj_mspbuffer(JNIEnv *env)
{
	s_mspbuf_clazz = getClassByName(env, MXJ_MSPBUF_CLASSNAME);
	checkException(env);
    mxj_mspbuf_register_natives(env,s_mspbuf_clazz);
    checkException(env);
	
	if (!s_mxj_buf_ref) {
		mxj_buf_ref_initclass();
		s_mxj_buf_ref = (t_object *)mxj_buf_ref_new();
	}
}

void mxj_mspbuf_register_natives(JNIEnv *env, jclass clazz)
{
	int count=0;
	JNINativeMethod methods[] =
	{
		{ "getChannels", "(Ljava/lang/String;)I", mxj_mspbuf_numchans },
   		{ "getLength","(Ljava/lang/String;)D", mxj_mspbuf_get_length  },
   		{ "setLength","(Ljava/lang/String;ID)V", mxj_mspbuf_set_length },
 		{ "getFrames","(Ljava/lang/String;)J", mxj_mspbuf_get_frames },
  		{ "setFrames", "(Ljava/lang/String;IJ)V", mxj_mspbuf_set_frames },
 		{ "getSize","(Ljava/lang/String;)J", mxj_mspbuf_get_size },
  		{ "setSize", "(Ljava/lang/String;IJ)V", mxj_mspbuf_set_size },
 		{ "peek","(Ljava/lang/String;IJ)F",mxj_mspbuf_peek },
 		{ "peek","(Ljava/lang/String;IJJ)[F",mxj_mspbuf_peek_region },
 		{ "peek","(Ljava/lang/String;I)[F",mxj_mspbuf_peek_channel },
 		{ "peek","(Ljava/lang/String;)[F",mxj_mspbuf_peek_all },
 		{ "poke","(Ljava/lang/String;IJF)V",mxj_mspbuf_poke },
   		{ "poke","(Ljava/lang/String;IJ[F)V",mxj_mspbuf_poke_region },
   		{ "poke","(Ljava/lang/String;I[F)V",mxj_mspbuf_poke_channel },
   		{ "poke","(Ljava/lang/String;[F)V",mxj_mspbuf_poke_all },
   		{ NULL, NULL, NULL }
	};
	
	while (methods[count].name) count++;
	
	//When adding methods dont forget to update the number of methods being passed to RegisterNatives
	MXJ_JNI_CALL(env,RegisterNatives)(env,clazz,methods,count);
	checkException(env);
}
